/*
===============================================================================

Name    :   g_network.cpp

Purpose :   NETORKING!!!

Comments:   this code is messy sloppy and bad, it was hacked together from
            the networking classes that i made for another app, the code
            here is poorly written and hardly intelligble, especially considering
            the lack of comments

            networking was mostly an afterthought for this program, you might
            see that there really isn't any sort of server-client architecture.
            basically clients are their own mini-server, where the real server
            gives it UNCOMPRESSED information EVERY FRAME for EVERY OBJECT

===============================================================================
*/

#include "local.h"
#pragma hdrstop

extern cvar_t   *g_upgrade_frac;
extern cvar_t   *g_upgrade_penalty;
extern cvar_t   *g_upgrade_min;

cvar_t          *net_master;            //  master server ip/hostname
cvar_t          *net_serverName;        //  name of local server

#define PROTOCOL_VERSION    3

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

void session::stop_client ()
{
    if ( _multiplayer && !_multiserver && _multiplayer_active )
    {
        _netchan.message.WriteByte( clc_disconnect );
        _netchan.Transmit( _netchan.message.nCurSize, _netchan.messagebuf );
        _netchan.message.Clear( );
    }

    _world.reset();

    _multiplayer = false;
    _multiplayer_active = false;

    _game_active = false;
    _menu_active = true;
}

/*
===========================================================

Name    :   m_GetPackets    m_GetFrame  m_WriteFrame    m_SendPackets

===========================================================
*/

void session::get_packets ()
{
    netsock_t   socket;
    client_t    *cl;
    int         i;

    float       time = g_Application->get_time( );

    if ( _multiserver )
        socket = NS_SERVER;
    else
        socket = NS_CLIENT;

    while ( pNet->Get( socket, &_netfrom, &_netmsg ) )
    {
        if ( *(int *)_netmsg.pData == -1 )
        {
            connectionless( socket );
            continue;
        }

        if ( socket == NS_SERVER )
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

void session::connectionless (netsock_t socket)
{
    static char* cmd; 

    _netmsg.Begin( );
    _netmsg.ReadLong( );

    cmd = _netstring = _netmsg.ReadString( );

    if ( socket == NS_SERVER )
    {
        if ( (strstr( cmd, "info" ) ) )
            info_send( );
        else if ( (strstr( cmd, "connect" ) ) )
            client_connect( );
    }
    else if ( socket == NS_CLIENT )
    {
        if ( (strstr( cmd, "info" ) ) )
            info_get( );
        else if ( (strstr( cmd, "connect" ) ) )
            connect_ack( );
        else if ( (strstr( cmd, "fail" ) ) )
            read_fail( );
    }
}

void session::packet (netsock_t socket)
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

        if ( socket == NS_SERVER )
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
        else if ( socket == NS_CLIENT )
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

void session::broadcast_print (char const* message)
{
    netmsg_t    netmsg;
    byte        netmsgbuf[MAX_MSGLEN];

    netmsg.Init( netmsgbuf, MAX_MSGLEN );
        
    netmsg.WriteByte( svc_message );
    netmsg.WriteString( message );

    broadcast( netmsg.nCurSize, netmsgbuf );
}

#define CLAMP_TIME  10.0f

void session::get_frame ()
{
    int             i;
    int             readbyte;
    int             framenum;

    _world.reset( );

    framenum = _netmsg.ReadLong( );

    if ( framenum < cls.last_frame )
        return;

    cls.last_frame = framenum;
    _framenum = framenum;

    //
    // allow some leeway for arriving packets, if they exceed it
    // clamp the time so that the velocity lerping doesn't goof up
    //

    _frametime = (float )_framenum * FRAMEMSEC;

    i = 0;
    while ( true )
    {
        readbyte = _netmsg.ReadByte( );
        if ( !readbyte )
            break;

        i = _netmsg.ReadByte( );

        _players[i]->_old_position = _players[i]->get_position();
        _players[i]->_old_rotation = _players[i]->get_rotation();
        _players[i]->_old_turret_rotation  = _players[i]->_turret_rotation;

        _players[i]->set_position(_netmsg.ReadVector());
        _players[i]->set_linear_velocity(_netmsg.ReadVector());
        _players[i]->set_rotation(_netmsg.ReadFloat());
        _players[i]->set_angular_velocity(_netmsg.ReadFloat());
        _players[i]->_turret_rotation = _netmsg.ReadFloat( );
        _players[i]->_turret_velocity = _netmsg.ReadFloat( );

        _players[i]->_damage = _netmsg.ReadFloat( );
        _players[i]->_fire_time = _netmsg.ReadFloat( );

        //m_World.AddObject( &m_Players[i] );

        // this is normally run on cTank::Think but is this 
        // not called on clients and must be called here
        _players[i]->update_sound( );


        //readbyte = m_netmsg.ReadByte( );

        //if ( readbyte )
        //{
        //    if ( m_Players[i].m_Bullet.bInGame ) {
        //        m_Players[i].m_Bullet.oldPos = m_Players[i].m_Bullet.get_position();
        //    } else {
        //        m_Players[i].m_Bullet.oldPos = m_Players[i].oldPos;
        //    }

        //    m_Players[i].m_Bullet.set_position(m_netmsg.ReadVector());
        //    m_Players[i].m_Bullet.set_linear_velocity(m_netmsg.ReadVector());

        //    m_World.AddObject( &m_Players[i].m_Bullet );

        //    m_Players[i].m_Bullet.bInGame = true;
        //} else {
        //    m_Players[i].m_Bullet.bInGame = false;
        //}
    }

    return;
}

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

    return;
}

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

/*
===========================================================

Name    :   m_ClientConnect m_ClientDisconnect  m_ClientPacket

===========================================================
*/

void session::connect_to_server (int index)
{
    // ask server for a connection

    stop_client( );
    stop_server( );

    if ( index > 0 )
        _netserver = cls.servers[index].address;

    _netserver.type = NA_IP;
    if ( !_netserver.port )
        _netserver.port = BIG_SHORT( PORT_SERVER );

    pNet->Print( NS_CLIENT, _netserver, va( "connect %i %s %i", PROTOCOL_VERSION, cls.name, _netchan.netport ) );
}

void session::connect_ack ()
{
    char    tempbuf[32];

    // server has ack'd our connect

    sscanf( _netstring, "%s %i", tempbuf, &cls.number );

    _netchan.Setup( NS_CLIENT, _netserver );

    _frametime = 0.0f;
    cls.last_frame = 0;

    _multiplayer = true;      // wtf are all these bools?
    _multiplayer_active = true;      // i dont even remember

    _menu_active = false;      // so you're totally screwed
    _game_active = true;       // if you ever want to know

    _players[cls.number]->_color.r = cls.color.r;
    _players[cls.number]->_color.g = cls.color.g;
    _players[cls.number]->_color.b = cls.color.b;

    _clients[cls.number].upgrades = 0;
    _clients[cls.number].damage_mod = 1.0f;
    _clients[cls.number].armor_mod = 1.0f;
    _clients[cls.number].refire_mod = 1.0f;
    _clients[cls.number].speed_mod = 1.0f;

    svs.clients[cls.number].active = true;
    strcpy( svs.clients[cls.number].name, cls.name );

    write_info( cls.number, &_netchan.message );
}

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

void session::read_fail ()
{
    textutils_c text;

    text.parse( _netstring );

    write_message( va( "Failed to connect: %s", text.argv(1) ) );
}

void session::write_info (int client, netmsg_t *message)
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
        netmsg_t    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        netmsg.Init( msgbuf, MAX_MSGLEN );
        write_info( client, &netmsg );
        broadcast( netmsg.nCurSize, netmsg.pData );
    }
}

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

void session::client_command ()
{
    game::tank* tank;
    int     bits;

    bits = _netmsg.ReadByte( );
    tank = _players[_netclient];

    memset( tank->_keys, 0, sizeof(tank->_keys) );

    if ( bits & BIT(KEY_FORWARD) )
        tank->_keys[KEY_FORWARD] = true;
    if ( bits & BIT(KEY_BACK) )
        tank->_keys[KEY_BACK] = true;

    if ( bits & BIT(KEY_LEFT) )
        tank->_keys[KEY_LEFT] = true;
    if ( bits & BIT(KEY_RIGHT) )
        tank->_keys[KEY_RIGHT] = true;

    if ( bits & BIT(KEY_TLEFT) )
        tank->_keys[KEY_TLEFT] = true;
    if ( bits & BIT(KEY_TRIGHT) )
        tank->_keys[KEY_TRIGHT] = true;
    if ( bits & BIT(KEY_FIRE) )
        tank->_keys[KEY_FIRE] = true;
}

void session::client_send ()
{
    int     bits = 0;

    if ( _client_keys[KEY_FORWARD] == true )
        bits |= BIT(KEY_FORWARD);
    if ( _client_keys[KEY_BACK] == true )
        bits |= BIT(KEY_BACK);

    if ( _client_keys[KEY_LEFT] == true )
        bits |= BIT(KEY_LEFT);
    if ( _client_keys[KEY_RIGHT] == true )
        bits |= BIT(KEY_RIGHT);

    if ( _client_keys[KEY_TLEFT] == true )
        bits |= BIT(KEY_TLEFT);
    if ( _client_keys[KEY_TRIGHT] == true )
        bits |= BIT(KEY_TRIGHT);
    if ( _client_keys[KEY_FIRE] == true )
        bits |= BIT(KEY_FIRE);

    _netchan.message.WriteByte( clc_command );
    _netchan.message.WriteByte( bits );
}

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

void session::read_sound ()
{
    int sound_index = _netmsg.ReadLong();
    pSound->playSound(sound_index, vec3(0,0,0), 1.0f, 0.0f);
}

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

void session::read_effect ()
{
    int type = _netmsg.ReadByte();
    vec2 pos = _netmsg.ReadVector();
    vec2 vel = _netmsg.ReadVector();
    float strength = _netmsg.ReadFloat();

    _world.add_effect(static_cast<game::effect_type>(type), pos, vel, strength);
}

/*
===========================================================

Name    :   m_InfoAsk   m_InfoSend  m_InfoGet

===========================================================
*/

void session::info_ask ()
{
    netadr_t    addr;

    for ( int i=0 ; i<MAX_SERVERS ; i++ )
    {
        cls.servers[i].name[0] = 0;
        cls.servers[i].active = false;
    }

    cls.ping_time = _frametime;

    stop_client( );
    stop_server( );

    _have_server = false;

    //  ping master server
    pNet->StringToNet( net_master->getString( ), &addr );
    addr.type = NA_IP;
    addr.port = BIG_SHORT( PORT_SERVER );
    pNet->Print( NS_CLIENT, addr, "info" );

    //  ping local network
    addr.type = NA_BROADCAST;
    addr.port = BIG_SHORT( PORT_SERVER );

    pNet->Print( NS_CLIENT, addr, "info" );
}

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

void session::info_get ()
{
    int         i;

    for ( i=0 ; i<MAX_SERVERS ; i++ )
    {
        // already exists and active, abort
        if ( cls.servers[i].address == _netfrom && cls.servers[i].active )
            return;

        // slot taken, try next
        if ( cls.servers[i].active )
            continue;

        cls.servers[i].address = _netfrom;
        cls.servers[i].ping = _frametime - cls.ping_time;

        strcpy( cls.servers[i].name, _netstring + 5 );

        cls.servers[i].active = true;

        // old server code
        if ( !_have_server )
        {
            _netserver = _netfrom;
            _have_server = true;
        }

        return;
    }
}

char    *sz_upgrades[] = {
    "damage",
    "armor",
    "gunnery",
    "speed" };

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

void session::write_upgrade (int upgrade)
{
    if ( _multiserver )
    {
        _netclient = 0;
        read_upgrade( upgrade );
        return;
    }

    _netchan.message.WriteByte( clc_upgrade );
    _netchan.message.WriteByte( upgrade );
}

} // namespace game
