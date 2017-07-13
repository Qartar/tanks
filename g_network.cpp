// g_network.cpp
//

#include "local.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void session::get_packets ()
{
    network::socket   socket;
    client_t    *cl;
    int         i;

    float       time = g_Application->time();

    if ( _multiserver )
        socket = network::socket::server;
    else
        socket = network::socket::client;

    while ( pNet->get( socket, &_netfrom, &_netmsg ) )
    {
        if ( *(int *)_netmsg.data == -1 )
        {
            connectionless( socket );
            continue;
        }

        if ( socket == network::socket::server )
        {
            int     netport;

            _netmsg.begin( );
            _netmsg.read_long( );   // trash
            netport = _netmsg.read_short( );

            for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
            {
                if ( cl->local )
                    continue;
                if ( !cl->active )
                    continue;
                if ( cl->netchan.address != _netfrom )
                    continue;
                if ( cl->netchan.netport != netport )
                    continue;

                // found him
                _netclient = i;
                break;
            }

            if ( i == MAX_PLAYERS )
                continue;   // bad client

            cl->netchan.process( &_netmsg );
        }
        else
        {
            if ( _netfrom != _netserver )
                break;  // not from our server

            _netchan.process( &_netmsg );
        }

        packet( socket );
    }

    //
    // check for timeouts
    //

    if ( _multiserver )
    {
        for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
        {
            if ( cl->local )
                continue;

            if ( !cl->active )
                continue;

            if ( cl->netchan.last_received + 10000 < time )
            {
                cl->netchan.message.write_byte( svc_disconnect );
                cl->netchan.transmit( cl->netchan.message.bytes_written, cl->netchan.messagebuf );

                write_message( va("%s timed out.", cl->name ) );
                client_disconnect( i );
            }
        }
    }
    else if ( _multiplayer_active )
    {
        if ( _netchan.last_received + 10000 < time )
        {
            write_message( "Server timed out." );
            stop_client( );
        }
    }
}

//------------------------------------------------------------------------------
void session::connectionless (network::socket socket)
{
    static char* cmd; 

    _netmsg.begin( );
    _netmsg.read_long( );

    cmd = _netstring = _netmsg.read_string( );

    if ( socket == network::socket::server )
    {
        if ( (strstr( cmd, "info" ) ) )
            info_send( );
        else if ( (strstr( cmd, "connect" ) ) )
            client_connect( );
    }
    else if ( socket == network::socket::client )
    {
        if ( (strstr( cmd, "info" ) ) )
            info_get( );
        else if ( (strstr( cmd, "connect" ) ) )
            connect_ack( );
        else if ( (strstr( cmd, "fail" ) ) )
            read_fail( );
    }
}

//------------------------------------------------------------------------------
void session::packet (network::socket socket)
{
    int     net_cmd;
    char    *string;
    int     net_read;

    while ( true )
    {
        if ( _netmsg.bytes_read > _netmsg.bytes_written )
            break;

        net_cmd = _netmsg.read_byte( );

        if ( net_cmd == -1 )
            return;

        if ( socket == network::socket::server )
        {
            switch ( net_cmd )
            {
            case clc_command:
                client_command( );
                break;

            case clc_disconnect:
                write_message( va("%s disconnected.", svs.clients[_netclient].name ) );
                client_disconnect( _netclient );
                break;

            case clc_say:
                string = _netmsg.read_string( );
                write_message( va( "\\c%02x%02x%02x%s\\cx: %s",
                    (int )(_players[_netclient]->_color.r * 255),
                    (int )(_players[_netclient]->_color.g * 255),
                    (int )(_players[_netclient]->_color.b * 255),
                    svs.clients[_netclient].name, string ) ); 
                break;

            case clc_upgrade:
                net_read = _netmsg.read_byte( );
                read_upgrade( net_read );
                break;

            case svc_info:
                read_info( );
                break;

            default:
                return;
            }
        }
        else if ( socket == network::socket::client )
        {
            switch ( net_cmd )
            {
            case svc_disconnect:
                _multiplayer_active = false;
                write_message( "Disconnected from server." );
                stop_client( );
                break;
            case svc_message:
                string = _netmsg.read_string( );
                write_message( string );
                break;
            case svc_score:
                net_read = _netmsg.read_byte( );
                _score[net_read] = _netmsg.read_byte( );
                break;
            case svc_info:
                read_info( );
                break;
            case svc_frame:
                get_frame( );
                break;
            case svc_effect:
                read_effect( );
                break;
            case svc_sound:
                read_sound( );
                break;
            case svc_restart:
                _restart_time = _frametime + _netmsg.read_byte( ) * 1000; //   time is in msec?
                break;
            default:
                return;
            }
        }
    }
}

//------------------------------------------------------------------------------
void session::broadcast (int len, byte *data)
{
    int             i;
    client_t        *cl;

    for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
    {
        if ( cl->local )
            continue;
        if ( !cl->active )
            continue;

        cl->netchan.message.write( data, len );
    }
}

//------------------------------------------------------------------------------
void session::broadcast_print (char const* message)
{
    network::message    netmsg;
    byte        netmsgbuf[MAX_MSGLEN];

    netmsg.init( netmsgbuf, MAX_MSGLEN );
        
    netmsg.write_byte( svc_message );
    netmsg.write_string( message );

    broadcast( netmsg.bytes_written, netmsgbuf );
}

//------------------------------------------------------------------------------
void session::send_packets ()
{
    int         i;
    client_t    *cl;

    if ( !_multiplayer_active )
        return;

    if ( _multiserver )
        goto server;

    client_send( );    // write move commands

    if ( _netchan.message.bytes_written )
    {
        _netchan.transmit( _netchan.message.bytes_written, _netchan.messagebuf );
        _netchan.message.clear( );
    }

    return;

server:
    for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
    {
        if ( cl->local )
            continue;   // local

        if ( !cl->active )
            continue;

        if ( !cl->netchan.message.bytes_written )
            continue;

        cl->netchan.transmit( cl->netchan.message.bytes_written, cl->netchan.messagebuf );
        cl->netchan.message.clear( );
    }
}

//------------------------------------------------------------------------------
void session::read_fail ()
{
    textutils_c text;

    text.parse( _netstring );

    write_message( va( "Failed to connect: %s", text.argv(1) ) );
}

//------------------------------------------------------------------------------
void session::write_info (int client, network::message *message)
{
    message->write_byte( svc_info );
    message->write_byte( client );
    message->write_byte( svs.clients[client].active );
    message->write_string( svs.clients[client].name );

    message->write_byte( svs.clients[client].color.x * 255 );
    message->write_byte( svs.clients[client].color.y * 255 );
    message->write_byte( svs.clients[client].color.z * 255 );

    //  write extra shit

    message->write_byte( _clients[client].upgrades );
    message->write_byte( _clients[client].armor_mod * 10 );
    message->write_byte( _clients[client].damage_mod * 10 );
    message->write_byte( _clients[client].refire_mod * 10 );
    message->write_byte( _clients[client].speed_mod * 10 );

    // also write score

    message->write_byte( svc_score );
    message->write_byte( client );
    message->write_byte( _score[client] );
}

//------------------------------------------------------------------------------
void session::read_info ()
{
    int     client;
    int     active;
    char    *string;

    client = _netmsg.read_byte( );
    active = _netmsg.read_byte( );

    svs.clients[client].active = ( active == 1 );
    
    string = _netmsg.read_string( );
    strncpy( svs.clients[client].name, string, SHORT_STRING );

    svs.clients[client].color.x = _netmsg.read_byte( ) / 255.0f;
    svs.clients[client].color.y = _netmsg.read_byte( ) / 255.0f;
    svs.clients[client].color.z = _netmsg.read_byte( ) / 255.0f;

    if (_players[client]) {
        _players[client]->_color = svs.clients[client].color;
    }

    _clients[client].upgrades = _netmsg.read_byte( );
    _clients[client].armor_mod = _netmsg.read_byte( ) / 10.0f;
    _clients[client].damage_mod = _netmsg.read_byte( ) / 10.0f;
    _clients[client].refire_mod = _netmsg.read_byte( ) / 10.0f;
    _clients[client].speed_mod = _netmsg.read_byte( ) / 10.0f;

    if ( _multiserver || _dedicated )
    {
        network::message    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        netmsg.init( msgbuf, MAX_MSGLEN );
        write_info( client, &netmsg );
        broadcast( netmsg.bytes_written, netmsg.data );
    }
}

} // namespace game
