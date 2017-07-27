// g_network.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void session::get_packets ()
{
    network::socket* socket = svs.active ? &svs.socket : &cls.socket;
    network::message_storage message;
    network::address remote;

    while (socket->read(remote, message)) {
        _net_bytes[_framenum % _net_bytes.size()] += message.bytes_remaining();

        int prefix = message.read_long();
        if (prefix != network::channel::prefix) {
            message.rewind();
            if (socket == &svs.socket) {
                server_connectionless(remote, message);
            } else {
                client_connectionless(remote, message);
            }
            message.reset();
            continue;
        }

        if (socket == &svs.socket) {
            int netport = (word )message.read_short();

            for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
                if (svs.clients[ii].local)
                    continue;
                if (!svs.clients[ii].active)
                    continue;
                if (svs.clients[ii].netchan.address() != remote)
                    continue;
                if (svs.clients[ii].netchan.netport() != netport)
                    continue;

                // found him
                svs.clients[ii].netchan.process(message);
                server_packet(message, ii);
                break;
            }
        } else {
            message.read_short(); // skip netport

            if (remote != _netserver) {
                break;  // not from our server
            }
            _netchan.process(message);
            client_packet(message);
        }

        message.reset();
    }

    //
    // check for timeouts
    //

    float time = g_Application->time();

    if (svs.active) {
        for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
            if (svs.clients[ii].local || !svs.clients[ii].active) {
                continue;
            }

            if (svs.clients[ii].netchan.last_received() + 10.0f < time) {
                svs.clients[ii].netchan.write_byte(svc_disconnect);
                svs.clients[ii].netchan.transmit();

                write_message(va("%s timed out.", svs.clients[ii].info.name.data()));
                client_disconnect(ii);
            }
        }
    } else if (cls.active) {
        if (_netchan.last_received() + 10.0f < time) {
            write_message("Server timed out.");
            stop_client();
        }
    }
}

//------------------------------------------------------------------------------
void session::broadcast(int len, byte const* data)
{
    for (auto& cl : svs.clients) {
        if (!cl.local && cl.active) {
            cl.netchan.write(data, len);
        }
    }
}

//------------------------------------------------------------------------------
void session::broadcast(network::message& message)
{
    int length = message.bytes_remaining();
    byte const* data = message.read(length);

    broadcast(length, data);
}

//------------------------------------------------------------------------------
void session::broadcast_print (char const* message)
{
    network::message_storage netmsg;
        
    netmsg.write_byte( svc_message );
    netmsg.write_string( message );

    broadcast(netmsg);
}

//------------------------------------------------------------------------------
void session::send_packets ()
{
    if (svs.active) {
        for (auto& cl : svs.clients) {
            if (cl.local || !cl.active || !cl.netchan.bytes_remaining()) {
                continue;
            }

            cl.netchan.transmit();
            cl.netchan.reset();
        }
    } else if (cls.active) {
        client_send();

        if (_netchan.bytes_remaining()) {
            _netchan.transmit();
            _netchan.reset();
        }
    }
}

//------------------------------------------------------------------------------
void session::read_fail(char const* message_string)
{
    textutils_c text;

    text.parse(message_string);

    write_message(va("Failed to connect: %s", text.argv(1)));
}

//------------------------------------------------------------------------------
void session::write_info(network::message& message, int client)
{
    message.write_byte( svc_info );
    message.write_byte( client );
    message.write_byte( svs.clients[client].active );
    message.write_string( svs.clients[client].info.name.data() );

    message.write_float( svs.clients[client].info.color.r );
    message.write_float( svs.clients[client].info.color.g );
    message.write_float( svs.clients[client].info.color.b );

    message.write_byte( static_cast<int>(svs.clients[client].info.weapon ) );

    //  write extra shit

    message.write_byte( _clients[client].upgrades );
    message.write_byte( _clients[client].armor_mod * 10 );
    message.write_byte( _clients[client].damage_mod * 10 );
    message.write_byte( _clients[client].refire_mod * 10 );
    message.write_byte( _clients[client].speed_mod * 10 );

    // also write score

    message.write_byte( svc_score );
    message.write_byte( client );
    message.write_byte( _score[client] );
}

//------------------------------------------------------------------------------
void session::read_info(network::message& message)
{
    int client;
    int active;
    char const* string;

    client = message.read_byte();
    active = message.read_byte();

    svs.clients[client].active = (active == 1);

    string = message.read_string();
    strncpy(svs.clients[client].info.name.data(), string, SHORT_STRING);

    svs.clients[client].info.color.r = message.read_float();
    svs.clients[client].info.color.g = message.read_float();
    svs.clients[client].info.color.b = message.read_float();

    svs.clients[client].info.weapon = static_cast<weapon_type>(message.read_byte());

    game::tank* player = _world.player(client);
    if (player) {
        player->_color = color4(svs.clients[client].info.color);
        player->_weapon = svs.clients[client].info.weapon;
    }

    _clients[client].upgrades = message.read_byte( );
    _clients[client].armor_mod = message.read_byte( ) / 10.0f;
    _clients[client].damage_mod = message.read_byte( ) / 10.0f;
    _clients[client].refire_mod = message.read_byte( ) / 10.0f;
    _clients[client].speed_mod = message.read_byte( ) / 10.0f;

    // relay info to other clients
    if (svs.active) {
        network::message_storage netmsg;

        write_info(netmsg, client);
        broadcast(netmsg);
    }
}

} // namespace game
