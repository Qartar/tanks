// g_session.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "cm_keys.h"
#include "cm_parser.h"
#include "g_tank.h"

#include "resource.h"

#include <numeric>
// #todo: abstract XInput virtual keycodes
#include <XInput.h>

// global object
game::session* g_Game;

void find_server(bool connect);

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
session::session()
    : _menu_active(true)
    , _dedicated(false)
    , _upgrade_frac("g_upgradeFrac", 0.5f, config::archive|config::server, "upgrade fraction")
    , _upgrade_penalty("g_upgradePenalty", 0.2f, config::archive|config::server, "upgrade penalty")
    , _upgrade_min("g_upgradeMin", 0.2f, config::archive|config::server, "minimum upgrade fraction")
    , _upgrades("g_upgrades", true, config::archive|config::server, "enable upgrades")
    , _net_master("net_master", "oedhead.no-ip.org", config::archive, "master server hostname")
    , _net_server_name("net_serverName", "Tanks! Server", config::archive, "local server name")
    , _net_graph("net_graph", false, config::archive, "draw network usage graph")
    , _cl_name("ui_name", "", config::archive, "user info: name")
    , _cl_color("ui_color", "255 0 0", config::archive, "user info: color")
    , _cl_weapon("ui_weapon", 0, config::archive, "user info: weapon")
    , _timescale("timescale", 1.f, config::server, "")
    , _restart_time(time_value::zero)
    , _worldtime(time_value::zero)
    , _frametime(time_value::zero)
    , _framenum(0)
    , _cursor(0,0)
    , _show_cursor(true)
    , _num_messages(0)
    , _client_button_down(false)
    , _server_button_down(false)
    , _client_say(false)
    , _command_quit("quit", &session::command_quit)
    , _command_disconnect("disconnect", this, &session::command_disconnect)
    , _command_connect("connect", this, &session::command_connect)
{
    log::set(this);
    g_Game = this;
}

//------------------------------------------------------------------------------
result session::init (string::view cmdline)
{
    _renderer = g_Application->window()->renderer();
    {
        float num = _renderer->view().size.x * ((640.f - 8.f) / 640.f);
        float den = _renderer->monospace_size(" ").x;
        _console.resize(static_cast<std::size_t>(num / den));
    }

    // Hack for MAKEINTRESOURCE which uses an integer as a pointer value
    _menu_image = _renderer->load_image(string::view(MAKEINTRESOURCE(IDB_BITMAP1),
                                                     MAKEINTRESOURCE(IDB_BITMAP1)));

    for ( int i=0 ; i<MAX_MESSAGES ; i++ )
        memset( _messages[i].string, 0, MAX_STRING );

    for ( int i=0 ; i<MAX_SERVERS ; i++ )
    {
        cls.servers[i].active = false;
        memset( cls.servers[i].name, 0, 32 );
    }

    _score[0] = 0;
    _score[1] = 0;

    svs.active = false;
    svs.local = false;

    for (auto& cl : svs.clients) {
        cl.active = false;
        cl.local = false;
        cl.info.name.fill('\0');
        cl.info.color = color3(1,1,1);
        cl.info.weapon = weapon_type::cannon;
    }

    memset( _clientsay, 0, LONG_STRING );

    strcpy( svs.name, _net_server_name );

    cls.active = false;
    cls.local = false;
    cls.number = 0;

    _clients[0].input.bind({
        {'w', usercmdgen::button::forward},
        {'s', usercmdgen::button::back},
        {'a', usercmdgen::button::left},
        {'d', usercmdgen::button::right},
        {'j', usercmdgen::button::turret_left},
        {'l', usercmdgen::button::turret_right},
        {'k', usercmdgen::button::fire},
    });

    _clients[1].input.bind({
        {K_UPARROW, usercmdgen::button::forward},
        {K_DOWNARROW, usercmdgen::button::back},
        {K_LEFTARROW, usercmdgen::button::left},
        {K_RIGHTARROW, usercmdgen::button::right},
        {K_KP_LEFTARROW, usercmdgen::button::turret_left},
        {K_KP_RIGHTARROW, usercmdgen::button::turret_right},
        {K_KP_5, usercmdgen::button::fire},
    });

    init_client();

    _menu.init( );
    _world.init( );

    if (cmdline.contains("dedicated")) {
        _dedicated = true;
        start_server( );
    }

    // sound indices are shared over the network so sounds
    // need to be registed in the same order on all clients
    pSound->load_sound("assets/sound/tank_move.wav");
    pSound->load_sound("assets/sound/tank_idle.wav");
    pSound->load_sound("assets/sound/tank_explode.wav");
    pSound->load_sound("assets/sound/turret_move.wav");
    pSound->load_sound("assets/sound/blaster_fire.wav");
    pSound->load_sound("assets/sound/blaster_impact.wav");
    pSound->load_sound("assets/sound/cannon_fire.wav");
    pSound->load_sound("assets/sound/cannon_impact.wav");
    pSound->load_sound("assets/sound/missile_flight.wav");

    write_message( "Welcome to Tanks! Press F1 or LSHLDR for help." );

    return result::success;
}

//------------------------------------------------------------------------------
result session::shutdown()
{
    stop_client( );
    shutdown_client();

    _world.shutdown( );
    _menu.shutdown( );

    return result::success;
}

//------------------------------------------------------------------------------
result session::run_frame(time_delta time)
{
    get_packets( );

    _frametime += time;

    // step session

    if (_frametime > time_value(_framenum * FRAMETIME)) {
        _framenum++;
        _net_bytes[_framenum % _net_bytes.size()] = 0;
    }

    // step world

    if (!_menu_active || svs.active) {
        // clamp world step size
        _worldtime += std::min(time, FRAMETIME) * _timescale;

        if (_worldtime > time_value((1 + _world.framenum()) * FRAMETIME) && svs.active) {
            _world.run_frame();
            if (!svs.local) {
                write_frame();
            }
        }
    }

    send_packets( );

    // draw everything

    update_screen();

    // update sound

    pSound->update( );

    if (_restart_time != time_value::zero && (_frametime > _restart_time) && !_menu_active ) {
        restart();
    }

    return result::success;
}

//------------------------------------------------------------------------------
void session::update_screen()
{
    _renderer->begin_frame();

    draw_world();

    //
    // calculate view for ui
    //

    render::view view = {};

    view.size = vec2(640, 480);
    view.origin = view.size * 0.5f;
    _renderer->set_view(view);

    //
    // draw ui
    //

    draw_menu();

    draw_score();

    draw_messages();

    draw_console();

    draw_netgraph();

    _renderer->end_frame();
}

//------------------------------------------------------------------------------
void session::draw_menu()
{
    // update cursor visibility

    if (_menu_active || !_renderer->window()->fullscreen()) {
        if (!_show_cursor) {
            ShowCursor(TRUE);
            _show_cursor = true;
        }
    } else {
        if (_show_cursor) {
            ShowCursor(FALSE);
            _show_cursor = false;
        }
    }

    // draw menu

    if (_menu_active) {

        // draw splash image

        float aspect = float(_menu_image->width()) / float(_menu_image->height());
        vec2 size = vec2(_renderer->view().size.y * aspect, _renderer->view().size.y);
        _renderer->draw_image(_menu_image, (_renderer->view().size - size) * 0.5f, size);

        // draw menu items

        _menu.draw(_renderer);
    }
}

//------------------------------------------------------------------------------
void session::char_event(int key)
{
    if (_console.char_event(key) || _console.active()) {
        return;
    }

    if (_client_button_down) {
        if (_menu_active) {
            std::size_t len = strnlen(cls.info.name.data(), cls.info.name.size());

            if (key == K_BACKSPACE && len) {
                cls.info.name[len - 1] = 0;
            } else if (key == K_ENTER) {
                _client_button_down = false;
            } else if (key <= K_SPACE) {
                return;
            } else if (key >= K_BACKSPACE) {
                return;
            } else if (len < 13) {
                cls.info.name[len + 1] = 0;
                cls.info.name[len] = narrow_cast<char>(key);
            }
            return;
        } else {
            _client_button_down = false;
        }
    } else if (_server_button_down) {
        if (_menu_active) {
            std::size_t len = strnlen(svs.name, countof(svs.name));

            if (key == K_BACKSPACE && len) {
                svs.name[strlen(svs.name) - 1] = 0;
            } else if (key == K_ENTER) {
                _server_button_down = false;
            } else if (key < K_SPACE) {
                return;
            } else if (key >= K_BACKSPACE) {
                return;
            } else if (len < countof(svs.name)) {
                svs.name[len + 1] = 0;
                svs.name[len] = narrow_cast<char>(key);
            }

            return;
        } else {
            _server_button_down = false;
        }
    } else if (_client_say) {
        if (key == K_BACKSPACE) {
            _clientsay[strlen(_clientsay) - 1] = 0;
        } else if (key == K_ENTER && strlen(_clientsay)) {
            if (!_clientsay[0]) {
                return;
            }

            if (svs.active || cls.active) {
                // say it
                _netchan.write_byte(clc_say);
                _netchan.write_string(_clientsay);

                if (svs.active && !svs.local) {
                    if (_dedicated) {
                        write_message(va("[Server]: %s", _clientsay));
                    } else {
                        write_message(va("^%x%x%x%s^xxx: %s",
                            (int)(cls.info.color.r * 15.5f),
                            (int)(cls.info.color.g * 15.5f),
                            (int)(cls.info.color.b * 15.5f),
                            cls.info.name.data(), _clientsay));
                    }
                }
            }

            memset(_clientsay, 0, LONG_STRING);
            _client_say = false;
        } else if (key == K_ENTER) {
            _client_say = false;
        } else if (key == K_ESCAPE) {
            memset(_clientsay, 0, LONG_STRING);
            _client_say = false;
        } else if (key < K_SPACE) {
            return;
        } else if (key > K_BACKSPACE) {
            return;
        } else if (strlen(_clientsay) < LONG_STRING) {
            _clientsay[strlen(_clientsay) + 1] = 0;
            _clientsay[strlen(_clientsay)] = narrow_cast<char>(key);
        }

        return;
    } else if (key == K_ENTER) {
        _client_say = true;
        return;
    } else if (key == '/') {
        _client_say = true;
        _clientsay[0] = '/';
        return;
    }
}

//------------------------------------------------------------------------------
void session::key_event(int key, bool down)
{
    if (_console.key_event(key, down) || _console.active()) {
        return;
    }

    if (_menu_active) {
        if (_menu.key_event(key, down)) {
            return;
        }
    }

    if (key == K_PGDN && down) {
        time_value time = g_Application->time();

        for (int i = 0; i < MAX_MESSAGES; i++) {
            _messages[i].time = time;
        }
    }

    // user commands here

    if (!_dedicated) {
        for (int ii = 0; ii < MAX_PLAYERS; ++ii) {
            game::tank* player = _world.player(ii);
            if (player && _clients[ii].input.key_event(key, down)) {
                player->update_usercmd(_clients[ii].input.generate());
            }
        }
    }

    if (down) {
        switch (key) {
            case K_F1:
            case VK_PAD_LSHOULDER:
                write_message_client("");
                write_message_client("----- TANKS HELP -----");
                write_message_client("note: pressing PGDN will refresh the message log");
                write_message_client("");
                write_message_client("  Each player commands an entire tank using the keyboard.");
                write_message_client("The following keys are using in multiplayer mode: ");
                write_message_client("W - Forward");
                write_message_client("S - Backward");
                write_message_client("A - Turns tank left");
                write_message_client("D - Turns tank right");
                write_message_client("J - Turns turret left");
                write_message_client("L - Turns turret right");
                write_message_client("K - Fire main gun");
                write_message_client("  Shots struck in the rear will do full damage (one shot");
                write_message_client("kill with no upgrades), the sides will do 1/2 damage, and");
                write_message_client("shots to the front will do 1/3 normal damage.");
                write_message_client("  You can change your nick and the color of your tank in");
                write_message_client("the Game Options menu. You can toggle the menu at any");
                write_message_client("time by pressing the ESC key.");
                write_message_client("  Every 10 kills you achieve in multiplayer you will be");
                write_message_client("prompted to upgrade your tank, you can see more about");
                write_message_client("upgrades by pressing F9 or RSHLDR");
                write_message_client("");
                break;

                //
                //  upgrades bullshit
                //

            case '1':
            case VK_PAD_A:
                if (_clients[cls.number].upgrades) {
                    write_upgrade(0);
                }
                break;

            case '2':
            case VK_PAD_B:
                if (_clients[cls.number].upgrades) {
                    write_upgrade(1);
                }
                break;

            case '3':
            case VK_PAD_X:
                if (_clients[cls.number].upgrades) {
                    write_upgrade(2);
                }
                break;

            case '4':
            case VK_PAD_Y:
                if (_clients[cls.number].upgrades) {
                    write_upgrade(3);
                }
                break;

            case K_F9:
            case VK_PAD_RSHOULDER:
                write_message_client("");
                write_message_client("---- UPGRADES HELP ----");
                write_message_client("  Upgrades are given every ten kills you achieve. The");
                write_message_client("categories you can upgrade in are the following: ");
                write_message_client("(1)(^8f8A^xxx) Damage - weapon damage");
                write_message_client("(2)(^d4dB^xxx) Armor - damage absorption");
                write_message_client("(3)(^44fX^xxx) Gunnery - fire rate");
                write_message_client("(4)(^ff0Y^xxx) Speed - tank speed");
                write_message_client("  To upgrade your tank, press the number associated with");
                write_message_client("the upgrade when you have upgrades available to you. You");
                write_message_client("should note that when you upgrade your tank a penalty");
                write_message_client("will be taken from a complementary category. Damage");
                write_message_client("goes with Gunnery, and Speed with Armor. However, when");
                write_message_client("you upgrade you will see a net increase in your tanks");
                write_message_client("performance.");
                write_message_client("");
                break;

            default:
                break;
        }
    }

    if (!down) {
        return;
    }

    // menu commands

    if (key == K_ESCAPE) {
        _menu_active ^= 1;
        return;
    }

    if (key == K_F2) {
        byte    msg[2];

        msg[0] = svc_restart;
        msg[1] = 5;

        broadcast(2, msg);

        _restart_time = _frametime + RESTART_TIME;
        return;
    }
}

//------------------------------------------------------------------------------
void session::cursor_event(vec2 position)
{
    vec2i size = g_Application->window()->size();
    _cursor.x = static_cast<int>(position.x * 640 / size.x);
    _cursor.y = static_cast<int>(position.y * 480 / size.y);

    if (_menu_active) {
        _menu.cursor_event(_cursor);
    }
}

//------------------------------------------------------------------------------
void session::gamepad_event(int index, gamepad const& pad)
{
    if (!_dedicated) {
        game::tank* player = _world.player(index);
        if (player) {
            _clients[index].input.gamepad_event(pad);
            player->update_usercmd(_clients[index].input.generate());
        }
    }
}

//------------------------------------------------------------------------------
void session::add_score(std::size_t player_index, int score)
{
    player_index = clamp<std::size_t>(player_index,0,MAX_PLAYERS);

    _score[player_index] += score;

    if (svs.active || cls.active) {
        if ( _score[player_index] % 10 == 0 ) {
            if ( _upgrades ) {
                _clients[player_index].upgrades++;
            }
        }
    }

    if (svs.active && !svs.local) {
        network::message_storage netmsg;

        write_info(netmsg, player_index);
        broadcast(netmsg);
    }
}

//------------------------------------------------------------------------------
void session::draw_score ()
{
    int width = static_cast<int>(_renderer->view().size.x);
    int height = static_cast<int>(_renderer->view().size.y);

    // draw console/chat box

    if (_client_say) {
        _renderer->draw_string("say:", vec2(vec2i(width/4,height-16)), menu::colors[7]);
        _renderer->draw_string(_clientsay, vec2(vec2i(width/4+32,height-16)), menu::colors[7]);
    }

    // don't draw score if menu is active

    if (_menu_active) {
        return;
    }

    // show player if upgrades are available

    if (_clients[cls.number].upgrades) {
        int num = _clients[cls.number].upgrades;

        if (num > 1) {
            _renderer->draw_string(va( "you have %i upgrades waiting...", num), vec2(8,12), menu::colors[7]);
        } else {
            _renderer->draw_string("you have 1 upgrade waiting...", vec2(8,12), menu::colors[7]);
        }
        _renderer->draw_string("for help with upgrades press F9 or RSHLDR", vec2(8,24), menu::colors[7]);
    }

    // count active players and maximum name width

    int panel_width = 96;
    int active_count = 0;

    if (svs.active || cls.active) {
        for (auto const& cl : svs.clients) {
            if (cl.active) {
                int cl_width = static_cast<int>(_renderer->string_size(string::view(cl.info.name.data())).x);
                panel_width = std::max<int>(panel_width, cl_width + 40);
                active_count++;
            }
        }
    } else {
        active_count = 2;
    }

    // draw the score panel

    _renderer->draw_box(
        vec2(vec2i(panel_width, active_count*12 + 8)),
        vec2(vec2i(width - panel_width/2 - 16, active_count*6 + 20)),
        menu::colors[4]);

    _renderer->draw_box(
        vec2(vec2i(panel_width - 2, active_count*12 + 8 - 2)),
        vec2(vec2i(width - panel_width/2 - 16, active_count*6 + 20)),
        menu::colors[5]);

    // sort players by score

    std::array<int, MAX_PLAYERS> sort;
    std::iota(sort.begin(), sort.end(), 0);
    std::sort(sort.begin(), sort.end(), [this](auto lhs, auto rhs) {
        return _score[lhs] > _score[rhs];
    });

    // draw each player score

    for (int n = 0, ii = 0; ii < MAX_PLAYERS; ++ii) {
        if (!svs.clients[sort[ii]].active) {
            continue;
        }

        string::buffer score(va("%d", _score[sort[ii]]));
        int score_width = static_cast<int>(_renderer->string_size(score).x);

        _renderer->draw_box(vec2(7,7), vec2(vec2i(width - panel_width - 8, n*12 + 26)), color4(svs.clients[sort[ii]].info.color));
        _renderer->draw_string(string::view(svs.clients[sort[ii]].info.name.data()), vec2(vec2i(width - panel_width - 2, n*12 + 30)), menu::colors[7]);
        _renderer->draw_string(score, vec2(vec2i(width - score_width - 20, n*12 + 30)), menu::colors[7]);

        n++;
    }

    // draw restart timer

    if (_restart_time > _frametime) {
        int nTime = static_cast<int>((_restart_time - _frametime).to_microseconds() / 1000);
        _renderer->draw_string(va("Restart in... %i", nTime), vec2(vec2i(width/2-48,16+13)), menu::colors[7]);
    }
}

//------------------------------------------------------------------------------
void session::draw_netgraph()
{
    constexpr float width = 580.0f;
    constexpr float height = 48.0f;
    constexpr float base = 128.0f; // 10 kbps

    if (!_net_graph) {
        return;
    }

    float sum = 0;
    float max = 0;

    for (size_t ii = 1; ii < _net_bytes.size() && ii < _framenum; ++ii) {
        std::size_t value = _net_bytes[(_framenum - ii) % _net_bytes.size()];
        sum += value;
        max = std::max(max, static_cast<float>(value));
    }

    float avg = sum / (std::min<std::size_t>(_framenum, _net_bytes.size()) - 1);
    float scale = std::min<float>(height / base, height / max);

    //
    // draw graph lines
    //

    for (size_t ii = 1; ii < _net_bytes.size() && ii < _framenum; ++ii) {
        float x0 = width - (ii - 1) * width / (_net_bytes.size() - 2);
        float x1 = width - (ii - 0) * width / (_net_bytes.size() - 2);
        float y0 = 480.0f - _net_bytes[(_framenum - ii + 0) % _net_bytes.size()] * scale;
        float y1 = 480.0f - _net_bytes[(_framenum - ii - 1) % _net_bytes.size()] * scale;

        _renderer->draw_line(vec2(x0,y0), vec2(x1, y1), color4(1,1,1,1), color4(1,1,1,1));
    }

    //
    // draw guide lines
    //

    {
        float ymax = std::min<float>(478.0f, 480.0f - max * scale);
        float yavg = std::min<float>(478.0f, 480.0f - avg * scale);
        float alpha_avg = std::min<float>(1, std::abs(ymax - yavg) * 0.1f);

        _renderer->draw_line(vec2(width, 480.0f), vec2(width, ymax - 8.0f), color4(1,1,1,1), color4(1,1,1,1));
        _renderer->draw_line(vec2(0, ymax), vec2(width, ymax), color4(1,1,1,1), color4(1,1,1,1));
        _renderer->draw_line(vec2(0, yavg), vec2(width, yavg), color4(0.5f,1,0.75f,alpha_avg), color4(0.5f,1,0.7f,alpha_avg));

        string::buffer smax(va("%0.1f kbps", CHAR_BIT * max / (FRAMETIME.to_seconds() * 1024.0f)));
        string::buffer savg(va("%0.1f kbps", CHAR_BIT * avg / (FRAMETIME.to_seconds() * 1024.0f)));

        _renderer->draw_string(smax, vec2(638.0f - _renderer->string_size(smax).x, ymax), color4(1,1,1,1));
        _renderer->draw_string(savg, vec2(638.0f - _renderer->string_size(savg).x, yavg), color4(1,1,1,alpha_avg));
    }
}

//------------------------------------------------------------------------------
void session::reset()
{
    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        _score[i] = 0;

        _clients[i].armor_mod = 1.0f;
        _clients[i].damage_mod = 1.0f;
        _clients[i].refire_mod = 1.0f;
        _clients[i].speed_mod = 1.0f;
        _clients[i].upgrades = 0;
    }

    _world.reset( );
}

//------------------------------------------------------------------------------
void session::resume()
{
    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::new_game()
{
    _restart_time = time_value::zero;
    _world.clear_particles( );

    if (!svs.active) {
        return;
    }

    //
    //  reset world
    //

    _world.reset( );

    //
    //  reset scores
    //

    if (svs.local) {
        network::message_storage netmsg;

        for ( int i=0 ; i<MAX_PLAYERS ; i++ ) {
            netmsg.write_byte(svc_score);   //  score command
            netmsg.write_byte(i);           //  player index
            netmsg.write_byte(0);           //  current score
        }
        broadcast(netmsg);
    }

    //
    //  reset players
    //

    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        _clients[i].armor_mod = 1.0f;
        _clients[i].damage_mod = 1.0f;
        _clients[i].refire_mod = 1.0f;
        _clients[i].speed_mod = 1.0f;
        _clients[i].upgrades = 0;

        _score[ i ] = 0;

        if (svs.local && i > 1 )
            break;
        else if (svs.active && !svs.clients[i].active )
            continue;

        if (_restart_time == time_value::zero || !_world.player(i)) {
            spawn_player(i);
        }
    }

    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::restart()
{
    _restart_time = time_value::zero;

    if (!svs.active) {
        return;
    }

    for (int ii = 0; ii < MAX_PLAYERS; ++ii) {
        if (svs.local && ii > 1) {
            break;
        } else if (svs.local && !svs.clients[ii].active) {
            continue;
        }

        assert(_world.player(ii) != nullptr);
        _world.player(ii)->respawn();
    }

    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::spawn_player(std::size_t num)
{
    //
    //  initialize tank object
    //

    assert(_world.player(num) == nullptr);
    game::tank* player = _world.spawn_player(num);
    player->_color = color4(svs.clients[num].info.color);
    player->_weapon = svs.clients[num].info.weapon;
    player->_client = _clients + num;

    player->respawn();

    //
    //  initialize stats
    //

    _score[num] = 0;

    _clients[num].armor_mod = 1.0f;
    _clients[num].damage_mod = 1.0f;
    _clients[num].refire_mod = 1.0f;
    _clients[num].speed_mod = 1.0f;
    _clients[num].upgrades = 0;
}

//------------------------------------------------------------------------------
result session::message(char const* format, ...)
{
    va_list list;
    char    string[MAX_STRING];

    va_start( list, format );
    vsprintf( string, format, list );
    va_end( list );

    strcpy( _messages[_num_messages].string, string );
    _messages[_num_messages].time = g_Application->time();

    _num_messages = (_num_messages+1)%MAX_MESSAGES;

    return result::success;
}

//------------------------------------------------------------------------------
void session::write_message (string::view message, bool broadcast)
{
    if (svs.active && broadcast) {
        broadcast_print( message );
    }

    memset( _messages[_num_messages].string, 0, MAX_STRING );
    strcpy( _messages[_num_messages].string, message );
    _messages[_num_messages].time = g_Application->time();

    _num_messages = (_num_messages+1)%MAX_MESSAGES;
}

//------------------------------------------------------------------------------
void session::draw_messages ()
{
    float       ypos;
    float       alpha;

    time_value time = g_Application->time();
    constexpr time_delta view_time = time_delta::from_seconds(15);
    constexpr time_delta fade_time = time_delta::from_seconds(3);

    ypos = _renderer->view().size.y - 36.f;

    for (int ii = _num_messages-1; ii != _num_messages; ii = (ii <= 0 ? MAX_MESSAGES-1 : ii-1)) {
        if (ii < 0) {
            continue;
        }

        if (_messages[ii].time + view_time > time) {
            alpha = (_messages[ii].time + (view_time - fade_time) > time ? 1.0f : (_messages[ii].time + view_time - time) / fade_time);

            _renderer->draw_string(_messages[ii].string, vec2(8.f,ypos), color4(1,1,1,alpha));

            ypos -= 12.f;
        }
    }
}

//------------------------------------------------------------------------------
void session::draw_console()
{
    if (!_console.height()) {
        return;
    }

    int width = static_cast<int>(_renderer->view().size.x);
    int height = static_cast<int>(_renderer->view().size.y);

    int yoffset = int(height * _console.height());
    _renderer->draw_box(
        vec2(vec2i(width, yoffset)),
        vec2(vec2i(width, yoffset) / 2),
        color4(.1f,.1f,.1f,.8f));

    int ystep = int(_renderer->monospace_size(" ").y);

    // draw input text
    {
        char buf[260] = "]";
        console_input const& input = _console.input();
        strncpy(buf + 1, input.begin(), input.end() - input.begin());
        _renderer->draw_monospace(buf, vec2(vec2i(4, yoffset - 8)), menu::colors[6]);
        // draw input cursor
        if ((int)(_frametime.to_seconds() * 2.5f) % 2 == 0) {
            buf[input.cursor() - input.begin() + 1] = '_';
            buf[input.cursor() - input.begin() + 2] = '\0';
            _renderer->draw_monospace(buf, vec2(vec2i(4, yoffset - 8)), menu::colors[6]);
        }
    }

    // draw console rows
    std::size_t num_rows = _console.num_rows();
    for (std::size_t ii = 0; ii + _console.scroll() < num_rows; ++ii) {
        int y = yoffset - narrow_cast<int>(ii + 1) * ystep - 8;
        if (y < 0) {
            break;
        }
        _renderer->draw_monospace(string::view(_console.get_row(ii + _console.scroll())),
                                  vec2(vec2i(4, y)),
                                  menu::colors[6]);
    }
}

//------------------------------------------------------------------------------
void session::command_quit(parser::text const&)
{
    PostQuitMessage(0);
}

//------------------------------------------------------------------------------
void session::command_disconnect(parser::text const&)
{
    stop_client();
    stop_server();
}

//------------------------------------------------------------------------------
void session::command_connect(parser::text const& args)
{
    if (args.tokens().size() > 2) {
    } else if (args.tokens().size() == 2) {
        connect_to_server(args.tokens()[1]);
    } else {
        connect_to_server(-1);
    }
}

//------------------------------------------------------------------------------
void session::print(log::level level, char const* msg)
{
    switch (level) {
        case log::level::message:
            _console.printf("%s", msg);
            break;

        case log::level::warning:
            _console.printf("^ff0warning^xxx: %s", msg);
            break;

        case log::level::error:
            _console.printf("^f00error^xxx: %s", msg);
            break;
    }
}

} // namespace game
