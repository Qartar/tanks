// g_client.cpp
//

#include "local.h"
#pragma hdrstop

cvar_t          *net_master;            //  master server ip/hostname

cvar_t  *cl_name = NULL;
cvar_t  *cl_color = NULL;

////////////////////////////////////////////////////////////////////////////////
namespace game {

const float colorMinFrac = 0.75f;

//------------------------------------------------------------------------------
void session::init_client()
{
    int length;
    float   csum;

    textutils_c text;

    _client_button_down = 0;
    _client_say = 0;

    cl_name = pVariable->Get( "ui_name", "", "string", CVAR_ARCHIVE, "user info: name" );
    cl_color = pVariable->Get( "ui_color", "255 0 0", "string", CVAR_ARCHIVE, "user info: color" );

    if ( strlen( cl_name->getString( )) )
        strcpy( cls.name, cl_name->getString( ) );
    else
        GetUserNameA( cls.name, (LPDWORD )&length );

    text.parse( cl_color->getString( ) );
    cls.color.r = (float )atoi(text.argv(0)) / 255.0f;
    cls.color.g = (float )atoi(text.argv(1)) / 255.0f;
    cls.color.b = (float )atoi(text.argv(2)) / 255.0f;

    csum = cls.color.r + cls.color.g + cls.color.b;
    if ( csum < colorMinFrac ) {
        if ( csum == 0.0f ) {
            cls.color.r = cls.color.g = cls.color.b = colorMinFrac * 0.334f;
        } else {
            float   invsum = colorMinFrac / csum;

            cls.color.r *= invsum;
            cls.color.g *= invsum;
            cls.color.b *= invsum;
        }
    }
}

//------------------------------------------------------------------------------
void session::shutdown_client()
{
    cl_name->setString( cls.name );
    cl_color->setString( va("%i %i %i", (int )(cls.color.r*255), (int )(cls.color.g*255), (int )(cls.color.b*255) ) );
}

//------------------------------------------------------------------------------
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

#define CLAMP_TIME  10.0f

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void session::connect_to_server (int index)
{
    // ask server for a connection

    stop_client( );
    stop_server( );

    if ( index > 0 )
        _netserver = cls.servers[index].address;

    _netserver.type = network::address_type::ip;
    if ( !_netserver.port )
        _netserver.port = BIG_SHORT( PORT_SERVER );

    pNet->Print( network::socket::client, _netserver, va( "connect %i %s %i", PROTOCOL_VERSION, cls.name, _netchan.netport ) );
}

//------------------------------------------------------------------------------
void session::connect_ack ()
{
    char    tempbuf[32];

    // server has ack'd our connect

    sscanf( _netstring, "%s %i", tempbuf, &cls.number );

    _netchan.Setup( network::socket::client, _netserver );

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

//------------------------------------------------------------------------------
void session::client_send ()
{
    game::usercmd cmd = _clients[0].input.generate();

    _netchan.message.WriteByte(clc_command);
    _netchan.message.WriteVector(cmd.move);
    _netchan.message.WriteVector(cmd.look);
    _netchan.message.WriteByte(static_cast<int>(cmd.action));
}

//------------------------------------------------------------------------------
void session::read_sound ()
{
    int sound_index = _netmsg.ReadLong();
    pSound->playSound(sound_index, vec3(0,0,0), 1.0f, 0.0f);
}

//------------------------------------------------------------------------------
void session::read_effect ()
{
    int type = _netmsg.ReadByte();
    vec2 pos = _netmsg.ReadVector();
    vec2 vel = _netmsg.ReadVector();
    float strength = _netmsg.ReadFloat();

    _world.add_effect(static_cast<game::effect_type>(type), pos, vel, strength);
}

//------------------------------------------------------------------------------
void session::info_ask ()
{
    network::address    addr;

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
    addr.type = network::address_type::ip;
    addr.port = BIG_SHORT( PORT_SERVER );
    pNet->Print( network::socket::client, addr, "info" );

    //  ping local network
    addr.type = network::address_type::broadcast;
    addr.port = BIG_SHORT( PORT_SERVER );

    pNet->Print( network::socket::client, addr, "info" );
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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
