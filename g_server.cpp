// g_server.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void session::start_server ()
{
    stop_client( );

    _multiplayer = true;
    _multiserver = true;
    _multiplayer_active = true;

    reset();

    // init local player

    if ( !_dedicated )
    {
        svs.clients[0].active = true;
        svs.clients[0].local = true;

        spawn_player(0);

        strncpy( svs.clients[0].name, cls.name, SHORT_STRING );
    }
    else
    {
        svs.clients[0].active = false;
        svs.clients[0].local = false;
    }

    _game_active = true;
    _menu_active = false;

    svs.active = true;
    _net_server_name = svs.name;

    _netchan.setup( network::socket::server, _netfrom );    // remote doesn't matter
}

//------------------------------------------------------------------------------
void session::stop_server ()
{
    int             i;
    client_t        *cl;

    if ( !_multiserver )
        return;

    svs.active = false;

    _multiplayer = false;
    _multiserver = false;
    _multiplayer_active = false;

    for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
    {
        _players[ i ] = nullptr;

        if ( cl->local )
            continue;
        if ( !cl->active )
            continue;

        client_disconnect( i );

        cl->netchan.message.write_byte( svc_disconnect );
        cl->netchan.transmit( cl->netchan.message.bytes_written, cl->netchan.messagebuf );
        cl->netchan.message.clear( );
    }

    _world.reset( );

    _game_active = false;
}

//------------------------------------------------------------------------------
void session::write_frame ()
{
    network::message    message;
    static byte messagebuf[MAX_MSGLEN];

    // HACK: update local players color here
    if ( svs.clients[0].local )
        _players[0]->_color = color4(cls.color);

    message.init( messagebuf, MAX_MSGLEN );
    message.clear( );

    _world.write_snapshot(message);

    broadcast( message.bytes_written, messagebuf );
}

//------------------------------------------------------------------------------
void session::client_connect ()
{
    // client has asked for connection

    int         i;
    client_t    *cl;
    int         netport, version;
    char        tempbuf[SHORT_STRING];
    char        clname[SHORT_STRING];

    network::message    message;
    byte        messagebuf[MAX_MSGLEN];

    message.init( messagebuf, MAX_MSGLEN );

    int width = 640;
    int height = 480;

    width -= SPAWN_BUFFER*2;
    height -= SPAWN_BUFFER*2;

    if ( !_multiserver )
        return;

    sscanf( _netstring, "%s %i %s %i", tempbuf, &version, clname, &netport );

    if ( version != PROTOCOL_VERSION )
    {
        pNet->print( network::socket::server, _netfrom, va("fail \"Bad protocol version: %i\"", version )  );
        return;
    }

    cl = NULL;
    for ( i=0 ; i<svs.max_clients ; i++ )
    {
        if ( !svs.clients[i].active )
        {
            cl = &svs.clients[i];
            break;
        }
        if ( svs.clients[i].netchan.address == _netfrom )
        {
            // this client is already connected
            if ( svs.clients[i].netchan.netport == netport )
                return;
        }
    }

    if ( !cl )  // couldn't find client slot
    {
        pNet->print( network::socket::server, _netfrom, "fail \"Server is full\"" );
        return;
    }

    // add new client

    cl->active = true;
    cl->local = false;
    cl->netchan.setup( network::socket::server, _netfrom, netport );

    strncpy( cl->name, clname, SHORT_STRING );

    pNet->print( network::socket::server, cl->netchan.address, va( "connect %i", i ) );

    // init their tank

    spawn_player(i);

    write_message( va( "%s connected.", cl->name ) );

    for ( i=0 ; i<svs.max_clients ; i++ )
    {
        if ( cl == &svs.clients[i] )
            continue;

        write_info( i, &cl->netchan.message );
    }

    return;
}

//------------------------------------------------------------------------------
void session::client_disconnect (int nClient)
{
    network::message    message;
    byte        messagebuf[MAX_MSGLEN];

    message.init( messagebuf, MAX_MSGLEN );

    if ( !svs.clients[nClient].active )
        return;

    _world.remove( _players[nClient] );
    _players[nClient] = nullptr;
    svs.clients[nClient].active = false;

    write_info( nClient, &message );
    broadcast( message.bytes_written, messagebuf );
}

//------------------------------------------------------------------------------
void session::client_command ()
{
    game::usercmd cmd{};

    cmd.move = _netmsg.read_vector();
    cmd.look = _netmsg.read_vector();
    cmd.action = static_cast<decltype(cmd.action)>(_netmsg.read_byte());

    if (_players[_netclient]) {
        _players[_netclient]->update_usercmd(cmd);
    }
}

//------------------------------------------------------------------------------
void session::write_sound(int sound_index)
{
    network::message    netmsg;
    static byte netmsgbuf[MAX_MSGLEN];

    if ( !_multiserver )
        return;

    netmsg.init( netmsgbuf, MAX_MSGLEN );
    netmsg.clear( );

    netmsg.write_byte( svc_sound );
    netmsg.write_long( sound_index );

    broadcast( netmsg.bytes_written, netmsgbuf );
}

//------------------------------------------------------------------------------
void session::write_effect (int type, vec2 pos, vec2 vel, float strength)
{
    network::message    netmsg;
    static byte netmsgbuf[MAX_MSGLEN];

    if (!_multiserver)
        return;

    netmsg.init(netmsgbuf, MAX_MSGLEN);
    netmsg.clear();

    netmsg.write_byte(svc_effect);
    netmsg.write_byte(type);
    netmsg.write_vector(pos);
    netmsg.write_vector(vel);
    netmsg.write_float(strength);

    broadcast(netmsg.bytes_written, netmsgbuf);
}

//------------------------------------------------------------------------------
void session::info_send ()
{
    int     i;

    if ( !svs.active )
        return;

    // check for an empty slot
    for ( i=0 ; i<MAX_PLAYERS ; i++ )
        if ( !svs.clients[i].active )
            break;

    // full, shhhhh
    if ( i == MAX_PLAYERS )
        return;

    pNet->print( network::socket::server, _netfrom, va( "info %s", svs.name) );
}

char    *sz_upgrades[] = {
    "damage",
    "armor",
    "gunnery",
    "speed" };

//------------------------------------------------------------------------------
void session::read_upgrade (int index)
{
    if ( _clients[_netclient].upgrades )
    {
        network::message    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        _clients[_netclient].upgrades--;
        if ( index == 0 )
        {
            _clients[_netclient].damage_mod += _upgrade_frac;
            _clients[_netclient].refire_mod -= _upgrade_penalty;
            if ( _clients[_netclient].refire_mod < _upgrade_min )
                _clients[_netclient].refire_mod = _upgrade_min;
        }
        else if ( index == 1 )
        {
            _clients[_netclient].armor_mod += _upgrade_frac;
            _clients[_netclient].speed_mod -= _upgrade_penalty;
            if ( _clients[_netclient].speed_mod < _upgrade_min )
                _clients[_netclient].speed_mod = _upgrade_min;
        }
        else if ( index == 2 )
        {
            _clients[_netclient].refire_mod += _upgrade_frac;
            _clients[_netclient].damage_mod -= _upgrade_penalty;
            if ( _clients[_netclient].damage_mod < _upgrade_min )
                _clients[_netclient].damage_mod = _upgrade_min;
        }
        else if ( index == 3 )
        {
            _clients[_netclient].speed_mod += _upgrade_frac;
            _clients[_netclient].armor_mod -= _upgrade_penalty;
            if ( _clients[_netclient].armor_mod < _upgrade_min )
                _clients[_netclient].armor_mod = _upgrade_min;
        }

        write_message( va( "\\c%02x%02x%02x%s\\cx has upgraded their %s!",
            (int )(_players[_netclient]->_color.r * 255),
            (int )(_players[_netclient]->_color.g * 255),
            (int )(_players[_netclient]->_color.b * 255),
            svs.clients[_netclient].name, sz_upgrades[index] ) ); 

        netmsg.init( msgbuf, MAX_MSGLEN );
        write_info( _netclient, &netmsg );
        broadcast( netmsg.bytes_written, netmsg.data );
    }
}

} // namespace game
