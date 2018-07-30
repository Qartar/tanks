// g_client.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_tank.h"

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

    if (strlen(_cl_name)) {
        strncpy(cls.info.name.data(), _cl_name, cls.info.name.size());
    } else {
        GetUserNameA(cls.info.name.data(), (LPDWORD )&length);
    }

    text.parse( _cl_color );
    cls.info.color.r = (float )atoi(text.argv(0)) / 255.0f;
    cls.info.color.g = (float )atoi(text.argv(1)) / 255.0f;
    cls.info.color.b = (float )atoi(text.argv(2)) / 255.0f;

    csum = cls.info.color.r + cls.info.color.g + cls.info.color.b;
    if ( csum < colorMinFrac ) {
        if ( csum == 0.0f ) {
            cls.info.color.r = cls.info.color.g = cls.info.color.b = colorMinFrac * 0.334f;
        } else {
            float   invsum = colorMinFrac / csum;

            cls.info.color.r *= invsum;
            cls.info.color.g *= invsum;
            cls.info.color.b *= invsum;
        }
    }

    cls.info.weapon = static_cast<weapon_type>((int)_cl_weapon);

    _net_bytes.fill(0);
}

//------------------------------------------------------------------------------
void session::shutdown_client()
{
    _cl_name = cls.info.name.data();
    _cl_color = va("%i %i %i", (int )(cls.info.color.r*255), (int )(cls.info.color.g*255), (int )(cls.info.color.b*255) );
    _cl_weapon = static_cast<int>(cls.info.weapon);
}

//------------------------------------------------------------------------------
void session::stop_client ()
{
    if (cls.active) {
        _netchan.write_byte(clc_disconnect);
        _netchan.transmit();
        _netchan.reset();
    }

    cls.active = false;
    cls.socket.close();

    _world.reset();

    _menu_active = true;
}

//------------------------------------------------------------------------------
void session::start_client_local()
{
    cls.active = true;
    cls.local = true;
}

//------------------------------------------------------------------------------
void session::client_connectionless(network::address const& remote, network::message& message)
{
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
    while (message.bytes_remaining()) {
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

            case svc_snapshot:
                read_snapshot(message);
                break;

            case svc_restart:
                _restart_time = _frametime + message.read_byte() * 1.0f;
                break;

            default:
                return;
        }
    }
}

//------------------------------------------------------------------------------
void session::read_snapshot(network::message& message)
{
    _world.read_snapshot(message);
    // gradually adjust client world time to match server
    // to compensate for variability in packet delivery
    float snapshot_time = _world.framenum() * FRAMETIME;
    _worldtime += (snapshot_time - _worldtime) * 0.1f;
    _net_bytes[++_framenum % _net_bytes.size()] = 0;
}

//------------------------------------------------------------------------------
void session::connect_to_server (int index)
{
    // ask server for a connection

    stop_client( );
    stop_server( );

    for (int ii = 0; ii < 16; ++ii) {
        if (cls.socket.open(network::socket_type::ipv6, PORT_CLIENT+ii)) {
            break;
        }
    }

    if ( index > 0 )
        _netserver = cls.servers[index].address;

    if ( !_netserver.port )
        _netserver.port = PORT_SERVER;

    cls.socket.printf(_netserver, "connect %i %s %i", PROTOCOL_VERSION, cls.info.name.data(), _netchan.netport());
}

//------------------------------------------------------------------------------
void session::connect_to_server (char const* address)
{
    // ask server for a connection

    stop_client( );
    stop_server( );

    for (int ii = 0; ii < 16; ++ii) {
        if (cls.socket.open(network::socket_type::ipv6, PORT_CLIENT+ii)) {
            break;
        }
    }

    if (!cls.socket.resolve(address, _netserver)) {
        write_message_client(va("Failed to resolve address: \"%s\"", address));
        return;
    }

    if ( !_netserver.port )
        _netserver.port = PORT_SERVER;

    cls.socket.printf(_netserver, "connect %i %s %i", PROTOCOL_VERSION, cls.info.name.data(), _netchan.netport());
}

//------------------------------------------------------------------------------
void session::connect_ack(char const* message_string)
{
    // server has ack'd our connect

    sscanf(message_string, "connect %i %f", &cls.number, &_worldtime);

    _netchan.setup( &cls.socket, _netserver );

    cls.active = true;

    _menu_active = false;

    _clients[cls.number].upgrades = 0;
    _clients[cls.number].damage_mod = 1.0f;
    _clients[cls.number].armor_mod = 1.0f;
    _clients[cls.number].refire_mod = 1.0f;
    _clients[cls.number].speed_mod = 1.0f;

    _clients[0].usercmd_time = 0.0f;

    svs.clients[cls.number].active = true;
    svs.clients[cls.number].info.name, cls.info.name;
    svs.clients[cls.number].info.color = cls.info.color;
    svs.clients[cls.number].info.weapon = cls.info.weapon;

    write_info(_netchan, cls.number);
}

//------------------------------------------------------------------------------
void session::client_send ()
{
    if (_frametime - _clients[0].usercmd_time < _clients[0].usercmd_rate) {
        return;
    }

    game::usercmd cmd = _clients[0].input.generate();
    _clients[0].usercmd_time = _frametime;

    _netchan.write_byte(clc_command);
    _netchan.write_vector(cmd.move);
    _netchan.write_vector(cmd.look);
    _netchan.write_byte(static_cast<int>(cmd.action));

    // check if user info has been changed
    if (!_menu_active) {
        if (strcmp(svs.clients[cls.number].info.name.data(), cls.info.name.data())
            || svs.clients[cls.number].info.color != cls.info.color
            || svs.clients[cls.number].info.weapon != cls.info.weapon) {

            strcpy(svs.clients[cls.number].info.name.data(), cls.info.name.data());
            svs.clients[cls.number].info.color = cls.info.color;
            svs.clients[cls.number].info.weapon = cls.info.weapon;
            write_info(_netchan, cls.number);
        }
    }
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

    for (int ii = 0; ii < 16; ++ii) {
        if (cls.socket.open(network::socket_type::ipv6, PORT_CLIENT+ii)) {
            break;
        }
    }

    //  ping master server
    if (cls.socket.resolve(_net_master, addr)) {
        if (addr.port == 0) {
            addr.port = PORT_SERVER;
        }
        cls.socket.printf(addr, "info");
    }

    //  ping local network
    addr.type = network::address_type::broadcast;
    addr.port = PORT_SERVER;

    cls.socket.printf(addr, "info");
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
        _netserver = remote;

        return;
    }
}

//------------------------------------------------------------------------------
void session::write_upgrade(int upgrade)
{
    if (svs.active) {
        read_upgrade(0, upgrade);
    } else {
        _netchan.write_byte(clc_upgrade);
        _netchan.write_byte(upgrade);
    }
}

//------------------------------------------------------------------------------
void session::draw_world()
{
    float const lerp = (_worldtime - _world.framenum() * FRAMETIME) / FRAMETIME;

    //
    // calculate view
    //

    vec2 world_size = _world.maxs() - _world.mins();
    vec2 world_center = _world.mins() + world_size * 0.5f;

    render::view view{};
    view.size = world_size;

    game::tank* player = _world.player(cls.number);
    if (cls.active && player) {
        vec2 position = player->get_position(lerp);
        vec2 view_mins = _world.mins() + view.size * 0.5f;
        vec2 view_maxs = _world.maxs() - view.size * 0.5f;

        view.origin.x = view_mins.x > view_maxs.x ? world_center.x
            : clamp(position.x, view_mins.x, view_maxs.x);

        view.origin.y = view_mins.y > view_maxs.y ? world_center.y
            : clamp(position.y, view_mins.y, view_maxs.y);
    } else {
        view.origin = world_center;
    }

    // draw world

    _renderer->set_view(view);

    if (cls.active) {
        _world.draw(_renderer, _worldtime);
    }

    // update sound listener

    pSound->set_listener(
        vec3(view.origin.x, view.origin.y, view.size.x * M_SQRT2),
        vec3(0,0,-1), // forward
        vec3(1,0,0), // right
        vec3(0,1,0) // up
    );
}

} // namespace game
