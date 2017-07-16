// g_client.cpp
//

#include "precompiled.h"
#pragma hdrstop

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

    if ( strlen(_cl_name) )
        strcpy( cls.name, _cl_name );
    else
        GetUserNameA( cls.name, (LPDWORD )&length );

    text.parse( _cl_color );
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
    _cl_name = cls.name;
    _cl_color = va("%i %i %i", (int )(cls.color.r*255), (int )(cls.color.g*255), (int )(cls.color.b*255) );
}

//------------------------------------------------------------------------------
void session::stop_client ()
{
    if ( _multiplayer && !_multiserver && _multiplayer_active )
    {
        _netchan.message.write_byte( clc_disconnect );
        _netchan.transmit( _netchan.message.bytes_written, _netchan.messagebuf );
        _netchan.message.clear( );
    }

    _world.reset();

    _multiplayer = false;
    _multiplayer_active = false;

    _game_active = false;
    _menu_active = true;
}

//------------------------------------------------------------------------------
void session::client_connectionless(network::address const& remote, network::message& message)
{
    message.begin();
    message.read_long();

    char const* message_string = message.read_string();

    if (strstr(message_string, "info")) {
        info_get(remote, message_string);
    } else if (strstr(message_string, "connect")) {
        connect_ack(message_string);
    } else if (strstr(message_string, "fail")) {
        read_fail(message_string);
    }
}

//------------------------------------------------------------------------------
void session::client_packet(network::message& message)
{
    while (message.bytes_read < message.bytes_written) {
        switch (message.read_byte()) {
            case svc_disconnect:
                write_message( "Disconnected from server." );
                stop_client();
                break;

            case svc_message:
                write_message(message.read_string());
                break;

            case svc_score: {
                int client = message.read_byte();
                int score = message.read_byte();
                _score[client] = score;
                break;
            }
            case svc_info:
                read_info(message);
                break;

            case svc_frame:
                get_frame(message);
                break;

            case svc_effect:
                read_effect(message);
                break;

            case svc_sound:
                read_sound(message);
                break;

            case svc_restart:
                _restart_time = _frametime + message.read_byte() * 1000;
                break;

            default:
                return;
        }
    }
}

//------------------------------------------------------------------------------
void session::get_frame(network::message& message)
{
    _world.read_snapshot(message);
    _framenum = _world.framenum();
    _frametime = (_framenum - 1) * FRAMEMSEC;
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

    pNet->print( network::socket::client, _netserver, va( "connect %i %s %i", PROTOCOL_VERSION, cls.name, _netchan.netport ) );
}

//------------------------------------------------------------------------------
void session::connect_ack(char const* message_string)
{
    // server has ack'd our connect

    sscanf(message_string, "connect %i", &cls.number);

    _netchan.setup( network::socket::client, _netserver );

    _frametime = 0.0f;
    cls.last_frame = 0;

    _multiplayer = true;      // wtf are all these bools?
    _multiplayer_active = true;      // i dont even remember

    _menu_active = false;      // so you're totally screwed
    _game_active = true;       // if you ever want to know

    _clients[cls.number].upgrades = 0;
    _clients[cls.number].damage_mod = 1.0f;
    _clients[cls.number].armor_mod = 1.0f;
    _clients[cls.number].refire_mod = 1.0f;
    _clients[cls.number].speed_mod = 1.0f;

    svs.clients[cls.number].active = true;
    strcpy( svs.clients[cls.number].name, cls.name );
    svs.clients[cls.number].color = cls.color;

    write_info(_netchan.message, cls.number);
}

//------------------------------------------------------------------------------
void session::client_send ()
{
    game::usercmd cmd = _clients[0].input.generate();

    _netchan.message.write_byte(clc_command);
    _netchan.message.write_vector(cmd.move);
    _netchan.message.write_vector(cmd.look);
    _netchan.message.write_byte(static_cast<int>(cmd.action));
}

//------------------------------------------------------------------------------
void session::read_sound(network::message& message)
{
    int asset = message.read_long();
    pSound->play(static_cast<sound::asset>(asset), vec3(0,0,0), 1.0f, 0.0f);
}

//------------------------------------------------------------------------------
void session::read_effect(network::message& message)
{
    int type = message.read_byte();
    vec2 pos = message.read_vector();
    vec2 vel = message.read_vector();
    float strength = message.read_float();

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
    pNet->string_to_address( _net_master, &addr );
    addr.type = network::address_type::ip;
    addr.port = BIG_SHORT( PORT_SERVER );
    pNet->print( network::socket::client, addr, "info" );

    //  ping local network
    addr.type = network::address_type::broadcast;
    addr.port = BIG_SHORT( PORT_SERVER );

    pNet->print( network::socket::client, addr, "info" );
}

//------------------------------------------------------------------------------
void session::info_get(network::address const& remote, char const* message_string)
{
    for (int ii=0; ii < MAX_SERVERS; ++ii) {
        // already exists and active, abort
        if ( cls.servers[ii].address == remote && cls.servers[ii].active )
            return;

        // slot taken, try next
        if ( cls.servers[ii].active )
            continue;

        cls.servers[ii].address = remote;
        cls.servers[ii].ping = _frametime - cls.ping_time;

        strcpy(cls.servers[ii].name, message_string + 5);

        cls.servers[ii].active = true;

        // old server code
        if (!_have_server) {
            _netserver = remote;
            _have_server = true;
        }

        return;
    }
}

//------------------------------------------------------------------------------
void session::write_upgrade(int upgrade)
{
    if (_multiserver) {
        read_upgrade(0, upgrade);
    } else {
        _netchan.message.write_byte(clc_upgrade);
        _netchan.message.write_byte(upgrade);
    }
}

} // namespace game
