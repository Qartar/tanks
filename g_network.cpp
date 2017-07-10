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

    float       time = g_Application->get_time( );

    if ( _multiserver )
        socket = network::socket::server;
    else
        socket = network::socket::client;

    while ( pNet->Get( socket, &_netfrom, &_netmsg ) )
    {
        if ( *(int *)_netmsg.pData == -1 )
        {
            connectionless( socket );
            continue;
        }

        if ( socket == network::socket::server )
        {
            int     netport;

            _netmsg.Begin( );
            _netmsg.ReadLong( );   // trash
            netport = _netmsg.ReadShort( );

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

            cl->netchan.Process( &_netmsg );
        }
        else
        {
            if ( _netfrom != _netserver )
                break;  // not from our server

            _netchan.Process( &_netmsg );
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
                cl->netchan.message.WriteByte( svc_disconnect );
                cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );

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

    _netmsg.Begin( );
    _netmsg.ReadLong( );

    cmd = _netstring = _netmsg.ReadString( );

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
        if ( _netmsg.nReadCount > _netmsg.nCurSize )
            break;

        net_cmd = _netmsg.ReadByte( );

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
                string = _netmsg.ReadString( );
                write_message( va( "\\c%02x%02x%02x%s\\cx: %s",
                    (int )(_players[_netclient]->_color.r * 255),
                    (int )(_players[_netclient]->_color.g * 255),
                    (int )(_players[_netclient]->_color.b * 255),
                    svs.clients[_netclient].name, string ) ); 
                break;

            case clc_upgrade:
                net_read = _netmsg.ReadByte( );
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
                string = _netmsg.ReadString( );
                write_message( string );
                break;
            case svc_score:
                net_read = _netmsg.ReadByte( );
                _score[net_read] = _netmsg.ReadByte( );
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
                _restart_time = _frametime + _netmsg.ReadByte( ) * 1000; //   time is in msec?
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

        cl->netchan.message.Write( data, len );
    }
}

//------------------------------------------------------------------------------
void session::broadcast_print (char const* message)
{
    network::message    netmsg;
    byte        netmsgbuf[MAX_MSGLEN];

    netmsg.Init( netmsgbuf, MAX_MSGLEN );
        
    netmsg.WriteByte( svc_message );
    netmsg.WriteString( message );

    broadcast( netmsg.nCurSize, netmsgbuf );
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

    if ( _netchan.message.nCurSize )
    {
        _netchan.Transmit( _netchan.message.nCurSize, _netchan.messagebuf );
        _netchan.message.Clear( );
    }

    return;

server:
    for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
    {
        if ( cl->local )
            continue;   // local

        if ( !cl->active )
            continue;

        if ( !cl->netchan.message.nCurSize )
            continue;

        cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );
        cl->netchan.message.Clear( );
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
    message->WriteByte( svc_info );
    message->WriteByte( client );
    message->WriteByte( svs.clients[client].active );
    message->WriteString( svs.clients[client].name );

    //  write extra shit

    message->WriteByte( _clients[client].upgrades );
    message->WriteByte( _clients[client].armor_mod * 10 );
    message->WriteByte( _clients[client].damage_mod * 10 );
    message->WriteByte( _clients[client].refire_mod * 10 );
    message->WriteByte( _clients[client].speed_mod * 10 );

    message->WriteByte( _players[client]->_color.r * 255 );
    message->WriteByte( _players[client]->_color.g * 255 );
    message->WriteByte( _players[client]->_color.b * 255 );

    // also write score

    message->WriteByte( svc_score );
    message->WriteByte( client );
    message->WriteByte( _score[client] );
}

//------------------------------------------------------------------------------
void session::read_info ()
{
    int     client;
    int     active;
    char    *string;

    client = _netmsg.ReadByte( );
    active = _netmsg.ReadByte( );

    svs.clients[client].active = ( active == 1 );
    
    string = _netmsg.ReadString( );
    strncpy( svs.clients[client].name, string, SHORT_STRING );

    _clients[client].upgrades = _netmsg.ReadByte( );
    _clients[client].armor_mod = _netmsg.ReadByte( ) / 10.0f;
    _clients[client].damage_mod = _netmsg.ReadByte( ) / 10.0f;
    _clients[client].refire_mod = _netmsg.ReadByte( ) / 10.0f;
    _clients[client].speed_mod = _netmsg.ReadByte( ) / 10.0f;

    _players[client]->_color.r = _netmsg.ReadByte( ) / 255.0f;
    _players[client]->_color.g = _netmsg.ReadByte( ) / 255.0f;
    _players[client]->_color.b = _netmsg.ReadByte( ) / 255.0f;

    if ( _multiserver || _dedicated )
    {
        network::message    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        netmsg.Init( msgbuf, MAX_MSGLEN );
        write_info( client, &netmsg );
        broadcast( netmsg.nCurSize, netmsg.pData );
    }
}

} // namespace game
