// g_server.cpp
//

#include "local.h"
#pragma hdrstop

extern cvar_t   *g_upgrade_frac;
extern cvar_t   *g_upgrade_penalty;
extern cvar_t   *g_upgrade_min;

cvar_t          *net_serverName;        //  name of local server

namespace game {

/*
===========================================================

Name    :   m_StartServer   m_StopServer

===========================================================
*/

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
    net_serverName->setString( svs.name );

    _netchan.Setup( NS_SERVER, _netfrom );    // remote doesn't matter
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

        cl->netchan.message.WriteByte( svc_disconnect );
        cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );
        cl->netchan.message.Clear( );
    }

    _world.reset( );

    _game_active = false;
}

//------------------------------------------------------------------------------
void session::write_frame ()
{
    netmsg_t    message;
    static byte messagebuf[MAX_MSGLEN];

    int             i;
    client_t        *cl;

    // HACK: update local players color here
    if ( svs.clients[0].local )
        _players[0]->_color = cls.color;

    message.Init( messagebuf, MAX_MSGLEN );
    message.Clear( );

    message.WriteByte( svc_frame );
    message.WriteLong( _framenum );
    for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
    {
        if ( !cl->active )
            continue;

        message.WriteByte( 1 );
        message.WriteByte( i );

        message.WriteVector( _players[i]->get_position() );
        message.WriteVector( _players[i]->get_linear_velocity() );
        message.WriteFloat( _players[i]->get_rotation() );
        message.WriteFloat( _players[i]->get_angular_velocity() );
        message.WriteFloat( _players[i]->_turret_rotation );
        message.WriteFloat( _players[i]->_turret_velocity );

        message.WriteFloat( _players[i]->_damage );
        message.WriteFloat( _players[i]->_fire_time );

        //if ( m_Players[i].m_Bullet.bInGame )
        //{
        //    message.WriteByte( 1 );

        //    message.WriteVector( m_Players[i].m_Bullet.get_position() );
        //    message.WriteVector( m_Players[i].m_Bullet.get_linear_velocity() );
        //}
        //else
        //    message.WriteByte( 0 );
    }

    message.WriteByte( 0 );

    broadcast( message.nCurSize, messagebuf );
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

    netmsg_t    message;
    byte        messagebuf[MAX_MSGLEN];

    message.Init( messagebuf, MAX_MSGLEN );

    int width = 640;
    int height = 480;

    width -= SPAWN_BUFFER*2;
    height -= SPAWN_BUFFER*2;

    if ( !_multiserver )
        return;

    sscanf( _netstring, "%s %i %s %i", tempbuf, &version, clname, &netport );

    if ( version != PROTOCOL_VERSION )
    {
        pNet->Print( NS_SERVER, _netfrom, va("fail \"Bad protocol version: %i\"", version )  );
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
        pNet->Print( NS_SERVER, _netfrom, "fail \"Server is full\"" );
        return;
    }

    // add new client

    cl->active = true;
    cl->local = false;
    cl->netchan.Setup( NS_SERVER, _netfrom, netport );

    strncpy( cl->name, clname, SHORT_STRING );

    pNet->Print( NS_SERVER, cl->netchan.address, va( "connect %i", i ) );

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
    netmsg_t    message;
    byte        messagebuf[MAX_MSGLEN];

    message.Init( messagebuf, MAX_MSGLEN );

    if ( !svs.clients[nClient].active )
        return;

    _world.remove( _players[nClient] );
    _players[nClient] = nullptr;
    svs.clients[nClient].active = false;

    write_info( nClient, &message );
    broadcast( message.nCurSize, messagebuf );
}

//------------------------------------------------------------------------------
void session::client_command ()
{
    game::usercmd cmd{};

    cmd.move = _netmsg.ReadVector();
    cmd.look = _netmsg.ReadVector();
    cmd.action = static_cast<decltype(cmd.action)>(_netmsg.ReadByte());

    if (_players[_netclient]) {
        _players[_netclient]->update_usercmd(cmd);
    }
}

//------------------------------------------------------------------------------
void session::write_sound(int sound_index)
{
    netmsg_t    netmsg;
    static byte netmsgbuf[MAX_MSGLEN];

    if ( !_multiserver )
        return;

    netmsg.Init( netmsgbuf, MAX_MSGLEN );
    netmsg.Clear( );

    netmsg.WriteByte( svc_sound );
    netmsg.WriteLong( sound_index );

    broadcast( netmsg.nCurSize, netmsgbuf );
}

//------------------------------------------------------------------------------
void session::write_effect (int type, vec2 pos, vec2 vel, float strength)
{
    netmsg_t    netmsg;
    static byte netmsgbuf[MAX_MSGLEN];

    if (!_multiserver)
        return;

    netmsg.Init(netmsgbuf, MAX_MSGLEN);
    netmsg.Clear();

    netmsg.WriteByte(svc_effect);
    netmsg.WriteByte(type);
    netmsg.WriteVector(pos);
    netmsg.WriteVector(vel);
    netmsg.WriteFloat(strength);

    broadcast(netmsg.nCurSize, netmsgbuf);
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

    pNet->Print( NS_SERVER, _netfrom, va( "info %s", svs.name) );
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
        netmsg_t    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        _clients[_netclient].upgrades--;
        if ( index == 0 )
        {
            _clients[_netclient].damage_mod += UPGRADE_FRAC;
            _clients[_netclient].refire_mod -= UPGRADE_PENALTY;
            if ( _clients[_netclient].refire_mod < UPGRADE_MIN )
                _clients[_netclient].refire_mod = UPGRADE_MIN;
        }
        else if ( index == 1 )
        {
            _clients[_netclient].armor_mod += UPGRADE_FRAC;
            _clients[_netclient].speed_mod -= UPGRADE_PENALTY;
            if ( _clients[_netclient].speed_mod < UPGRADE_MIN )
                _clients[_netclient].speed_mod = UPGRADE_MIN;
        }
        else if ( index == 2 )
        {
            _clients[_netclient].refire_mod += UPGRADE_FRAC;
            _clients[_netclient].damage_mod -= UPGRADE_PENALTY;
            if ( _clients[_netclient].damage_mod < UPGRADE_MIN )
                _clients[_netclient].damage_mod = UPGRADE_MIN;
        }
        else if ( index == 3 )
        {
            _clients[_netclient].speed_mod += UPGRADE_FRAC;
            _clients[_netclient].armor_mod -= UPGRADE_PENALTY;
            if ( _clients[_netclient].armor_mod < UPGRADE_MIN )
                _clients[_netclient].armor_mod = UPGRADE_MIN;
        }

        write_message( va( "\\c%02x%02x%02x%s\\cx has upgraded their %s!",
            (int )(_players[_netclient]->_color.r * 255),
            (int )(_players[_netclient]->_color.g * 255),
            (int )(_players[_netclient]->_color.b * 255),
            svs.clients[_netclient].name, sz_upgrades[index] ) ); 

        netmsg.Init( msgbuf, MAX_MSGLEN );
        write_info( _netclient, &netmsg );
        broadcast( netmsg.nCurSize, netmsg.pData );
    }
}

} // namespace game
