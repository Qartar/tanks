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

    for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
        svs.clients[ii].active = false;
        svs.clients[ii].local = false;
    }

    // init local player

    if (!_dedicated) {
        svs.clients[0].active = true;
        svs.clients[0].local = true;

        strncpy(svs.clients[0].name, cls.name, SHORT_STRING);
        svs.clients[0].color = cls.color;
        svs.clients[0].weapon = cls.weapon;

        spawn_player(0);
    }

    _game_active = true;
    _menu_active = false;

    svs.active = true;
    _net_server_name = svs.name;

    svs.socket.open(network::socket_type::ipv6, PORT_SERVER);
    _netchan.setup(&svs.socket, network::address{});
}

//------------------------------------------------------------------------------
void session::start_server_local()
{
    stop_client();

    _multiplayer = false;
    _multiserver = false;
    _multiplayer_active = false;

    svs.active = false;

    // init local players
    for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
        if (ii < 2) {
            svs.clients[ii].active = true;
            svs.clients[ii].local = true;

            fmt(svs.clients[ii].name, "Player %i", ii+1);
            svs.clients[ii].color = player_colors[ii];
        } else {
            svs.clients[ii].active = false;
            svs.clients[ii].local = false;
        }
    }

    new_game();
}

//------------------------------------------------------------------------------
void session::stop_server ()
{
    if (!_multiserver) {
        return;
    }

    svs.active = false;

    _multiplayer = false;
    _multiserver = false;
    _multiplayer_active = false;

    for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
        if (svs.clients[ii].local || !svs.clients[ii].active) {
            continue;
        }

        client_disconnect(ii);

        svs.clients[ii].netchan.message.write_byte(svc_disconnect);
        svs.clients[ii].netchan.transmit(svs.clients[ii].netchan.message);
        svs.clients[ii].netchan.message.reset();
    }

    svs.socket.close();

    _world.reset();

    _game_active = false;
}

//------------------------------------------------------------------------------
void session::server_connectionless(network::address const& remote, network::message& message)
{
    char const* message_string = message.read_string();

    if (strstr(message_string, "info")) {
        info_send(remote);
    } else if (strstr(message_string, "connect")) {
        client_connect(remote, message_string);
    }
}

//------------------------------------------------------------------------------
void session::server_packet(network::message& message, int client)
{
    while (message.bytes_remaining()) {
        switch (message.read_byte()) {
            case clc_command:
                client_command(message, client);
                break;

            case clc_disconnect:
                write_message(va("%s disconnected.", svs.clients[client].name ));
                client_disconnect(client);
                break;

            case clc_say:
                write_message(va( "\\c%02x%02x%02x%s\\cx: %s",
                    (int )(svs.clients[client].color.r * 255),
                    (int )(svs.clients[client].color.g * 255),
                    (int )(svs.clients[client].color.b * 255),
                    svs.clients[client].name, message.read_string())); 
                break;

            case clc_upgrade:
                read_upgrade(client, message.read_byte());
                break;

            case svc_info:
                read_info(message);
                break;

            default:
                return;
        }
    }
}

//------------------------------------------------------------------------------
void session::write_frame()
{
    network::message_storage message;

    _world.write_snapshot(message);

    broadcast(message);
}

//------------------------------------------------------------------------------
void session::client_connect(network::address const& remote, char const* message_string)
{
    // client has asked for connection
    if (!_multiserver) {
        return;
    }

    // ensure that this client hasn't already connected
    for (auto const& cl : svs.clients) {
        if (cl.active && cl.netchan.address() == remote) {
            return;
        }
    }

    // find an available client slot
    for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
        if (!svs.clients[ii].active) {
            return client_connect(remote, message_string, ii);
        }
    }

    svs.socket.printf(remote, "fail \"Server is full\"");
}

//------------------------------------------------------------------------------
void session::client_connect(network::address const& remote, char const* message_string, int client)
{
    auto& cl = svs.clients[client];
    int netport, version;

    sscanf(message_string, "connect %i %s %i", &version, cl.name, &netport);

    if (version != PROTOCOL_VERSION) {
        svs.socket.printf(remote, va("fail \"Bad protocol version: %i\"", version));
    } else {
        cl.active = true;
        cl.local = false;
        cl.netchan.setup(&svs.socket, remote, netport);

        svs.socket.printf(cl.netchan.address(), va("connect %i", client));

        // init their tank

        spawn_player(client);

        write_message(va("%s connected.", cl.name));

        // broadcast existing client information to new client
        for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
            if (&cl != &svs.clients[ii]) {
                write_info(cl.netchan.message, ii);
            }
        }
    }
}

//------------------------------------------------------------------------------
void session::client_disconnect(int client)
{
    network::message_storage message;

    if (!svs.clients[client].active) {
        return;
    }

    _world.remove_player(client);
    svs.clients[client].active = false;

    write_info(message, client);
    broadcast(message);
}

//------------------------------------------------------------------------------
void session::client_command(network::message& message, int client)
{
    game::usercmd cmd{};

    cmd.move = message.read_vector();
    cmd.look = message.read_vector();
    cmd.action = static_cast<decltype(cmd.action)>(message.read_byte());

    game::tank* player = _world.player(client);
    if (player) {
        player->update_usercmd(cmd);
    }
}

//------------------------------------------------------------------------------
void session::write_sound(int sound, vec2 position, float volume)
{
    network::message_storage netmsg;

    if (!_multiserver) {
        return;
    }

    netmsg.write_byte(svc_sound);
    netmsg.write_long(sound);
    netmsg.write_vector(position);
    netmsg.write_float(volume);

    broadcast(netmsg);
}

//------------------------------------------------------------------------------
void session::write_effect (int type, vec2 pos, vec2 vel, float strength)
{
    network::message_storage netmsg;

    if (!_multiserver) {
        return;
    }

    netmsg.write_byte(svc_effect);
    netmsg.write_byte(type);
    netmsg.write_vector(pos);
    netmsg.write_vector(vel);
    netmsg.write_float(strength);

    broadcast(netmsg);
}

//------------------------------------------------------------------------------
void session::info_send(network::address const& remote)
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

    svs.socket.printf(remote, va( "info %s", svs.name) );
}

char    *sz_upgrades[] = {
    "damage",
    "armor",
    "gunnery",
    "speed" };

//------------------------------------------------------------------------------
void session::read_upgrade(int client, int upgrade)
{
    if (_clients[client].upgrades) {
        network::message_storage netmsg;

        _clients[client].upgrades--;
        if (upgrade == 0) {
            _clients[client].damage_mod += _upgrade_frac;
            _clients[client].refire_mod -= _upgrade_penalty;
            if (_clients[client].refire_mod < _upgrade_min) {
                _clients[client].refire_mod = _upgrade_min;
            }
        } else if (upgrade == 1) {
            _clients[client].armor_mod += _upgrade_frac;
            _clients[client].speed_mod -= _upgrade_penalty;
            if (_clients[client].speed_mod < _upgrade_min) {
                _clients[client].speed_mod = _upgrade_min;
            }
        } else if (upgrade == 2) {
            _clients[client].refire_mod += _upgrade_frac;
            _clients[client].damage_mod -= _upgrade_penalty;
            if (_clients[client].damage_mod < _upgrade_min) {
                _clients[client].damage_mod = _upgrade_min;
            }
        } else if (upgrade == 3) {
            _clients[client].speed_mod += _upgrade_frac;
            _clients[client].armor_mod -= _upgrade_penalty;
            if (_clients[client].armor_mod < _upgrade_min) {
                _clients[client].armor_mod = _upgrade_min;
            }
        }

        write_message( va( "\\c%02x%02x%02x%s\\cx has upgraded their %s!",
            (int )(svs.clients[client].color.r * 255),
            (int )(svs.clients[client].color.g * 255),
            (int )(svs.clients[client].color.b * 255),
            svs.clients[client].name, sz_upgrades[upgrade] ) ); 

        write_info(netmsg, client);
        broadcast(netmsg);
    }
}

} // namespace game
