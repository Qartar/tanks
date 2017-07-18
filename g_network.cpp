// g_network.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void session::get_packets ()
{
    network::socket   socket;

    byte message_buf[MAX_MSGLEN];
    network::message message;

    message.init(message_buf, MAX_MSGLEN);

    if ( _multiserver )
        socket = network::socket::server;
    else
        socket = network::socket::client;

    network::address remote;

    while (pNet->get(socket, &remote, &message)) {
        if (*(int *)message.data == -1) {
            if (socket == network::socket::server) {
                server_connectionless(remote, message);
            } else {
                client_connectionless(remote, message);
            }
            continue;
        }

        if (socket == network::socket::server) {
            int     netport;

            message.begin();
            message.read_long();
            netport = message.read_short();

            for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
                if (svs.clients[ii].local)
                    continue;
                if (!svs.clients[ii].active)
                    continue;
                if (svs.clients[ii].netchan.address != remote)
                    continue;
                if (svs.clients[ii].netchan.netport != netport)
                    continue;

                // found him
                svs.clients[ii].netchan.process(&message);
                server_packet(message, ii);
                break;
            }
        } else {
            if (remote != _netserver) {
                break;  // not from our server
            }
            _netchan.process(&message);
            client_packet(message);
        }
    }

    //
    // check for timeouts
    //

    float time = g_Application->time();

    if (_multiserver) {
        for (std::size_t ii = 0; ii < svs.clients.size(); ++ii) {
            if (svs.clients[ii].local || !svs.clients[ii].active) {
                continue;
            }

            if (svs.clients[ii].netchan.last_received + 10000 < time) {
                svs.clients[ii].netchan.message.write_byte(svc_disconnect);
                svs.clients[ii].netchan.transmit(
                    svs.clients[ii].netchan.message.bytes_written,
                    svs.clients[ii].netchan.messagebuf);

                write_message(va("%s timed out.", svs.clients[ii].name));
                client_disconnect(ii);
            }
        }
    } else if (_multiplayer_active) {
        if (_netchan.last_received + 10000 < time) {
            write_message("Server timed out.");
            stop_client();
        }
    }
}

//------------------------------------------------------------------------------
void session::broadcast (int len, byte *data)
{
    for (auto& cl : svs.clients) {
        if (!cl.local && cl.active) {
            cl.netchan.message.write(data, len);
        }
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
    if (!_multiplayer_active) {
        return;
    }

    if (_multiserver) {
        for (auto& cl : svs.clients) {
            if (cl.local || !cl.active || !cl.netchan.message.bytes_written) {
                continue;
            }

            cl.netchan.transmit(cl.netchan.message.bytes_written, cl.netchan.messagebuf);
            cl.netchan.message.clear();
        }
    } else {
        client_send();

        if (_netchan.message.bytes_written) {
            _netchan.transmit(_netchan.message.bytes_written, _netchan.messagebuf);
            _netchan.message.clear();
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
    message.write_string( svs.clients[client].name );

    message.write_byte( svs.clients[client].color.r * 255 );
    message.write_byte( svs.clients[client].color.g * 255 );
    message.write_byte( svs.clients[client].color.b * 255 );

    message.write_byte( static_cast<int>(svs.clients[client].weapon ) );

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
    int     client;
    int     active;
    char    *string;

    client = message.read_byte( );
    active = message.read_byte( );

    svs.clients[client].active = ( active == 1 );

    string = message.read_string( );
    strncpy( svs.clients[client].name, string, SHORT_STRING );

    svs.clients[client].color.r = message.read_byte( ) / 255.0f;
    svs.clients[client].color.g = message.read_byte( ) / 255.0f;
    svs.clients[client].color.b = message.read_byte( ) / 255.0f;

    svs.clients[client].weapon = static_cast<weapon_type>(message.read_byte());

    game::tank* player = _world.player(client);
    if (player) {
        player->_color = color4(svs.clients[client].color);
        player->_weapon = svs.clients[client].weapon;
    }

    _clients[client].upgrades = message.read_byte( );
    _clients[client].armor_mod = message.read_byte( ) / 10.0f;
    _clients[client].damage_mod = message.read_byte( ) / 10.0f;
    _clients[client].refire_mod = message.read_byte( ) / 10.0f;
    _clients[client].speed_mod = message.read_byte( ) / 10.0f;

    // relay info to other clients
    if (_multiserver || _dedicated) {
        network::message    netmsg;
        byte        msgbuf[MAX_MSGLEN];

        netmsg.init(msgbuf, MAX_MSGLEN);
        write_info(netmsg, client);
        broadcast(netmsg.bytes_written, netmsg.data);
    }
}

} // namespace game
