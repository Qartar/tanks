// g_session.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "keys.h"
#include "resource.h"

#include <numeric>

// global object
vMain   *pMain;
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
    , _restart_time(0)
    , _worldtime(0)
    , _frametime(0)
    , _framenum(0)
    , _cursor(0,0)
    , _show_cursor(true)
    , _num_messages(0)
    , _client_button_down(false)
    , _server_button_down(false)
    , _client_say(false)
{
    g_Game = this;
    pMain = this;
}

//------------------------------------------------------------------------------
int session::init (char const *cmdline)
{
    _config = g_Application->config();
    _renderer = g_Application->window()->renderer();

    _menu_image = _renderer->load_image(MAKEINTRESOURCE(IDB_BITMAP1));

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

    if ( strstr( cmdline, "dedicated" ) )
    {
        _dedicated = true;
        start_server( );
    }

    for (int i=0 ; i<256 ; i++)
        _shift_keys[i] = i;
    for (int i='a' ; i<='z' ; i++)
        _shift_keys[i] = i - 'a' + 'A';
    _shift_keys['1'] = '!';
    _shift_keys['2'] = '@';
    _shift_keys['3'] = '#';
    _shift_keys['4'] = '$';
    _shift_keys['5'] = '%';
    _shift_keys['6'] = '^';
    _shift_keys['7'] = '&';
    _shift_keys['8'] = '*';
    _shift_keys['9'] = '(';
    _shift_keys['0'] = ')';
    _shift_keys['-'] = '_';
    _shift_keys['='] = '+';
    _shift_keys[','] = '<';
    _shift_keys['.'] = '>';
    _shift_keys['/'] = '?';
    _shift_keys[';'] = ':';
    _shift_keys['\''] = '"';
    _shift_keys['['] = '{';
    _shift_keys[']'] = '}';
    _shift_keys['`'] = '~';
    _shift_keys['\\'] = '|';

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

    write_message( "Welcome to Tanks! Press F1 for help." );

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int session::shutdown()
{
    stop_client( );
    shutdown_client();

    _world.shutdown( );
    _menu.shutdown( );

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int session::run_frame(float milliseconds)
{
    rand( );

    get_packets( );

    _frametime += milliseconds;

    // step session

    if (_frametime > _framenum * FRAMETIME) {
        _framenum++;
        _net_bytes[_framenum % _net_bytes.size()] = 0;
    }

    // step world

    if (!_menu_active || svs.active) {
        // clamp world step size
        _worldtime += std::min(milliseconds, FRAMETIME);

        if (_worldtime > (1 + _world.framenum()) * FRAMETIME && svs.active) {
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

    if ( _restart_time && (_frametime > _restart_time) && !_menu_active ) {
        restart();
    }

    return ERROR_NONE;
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
void session::key_event(unsigned char key, bool down)
{
    static bool shift = false;
    static bool ctrl = false;

    // mouse commands

    if (_menu_active) {
        if (_menu.key_event(key, down)) {
            return;
        }
    }

    if (key == K_SHIFT)
        shift = down;
    if (key == K_CTRL)
        ctrl = down;

    if ( _client_button_down )
    {
        if ( _menu_active )
        {
            if ( !down )
                return;

            if ( shift )
                key = _shift_keys[key];

            std::size_t len = strnlen(cls.info.name.data(), cls.info.name.size());

            if (key == K_BACKSPACE && len) {
                cls.info.name[len-1] = 0;
            } else if (key == K_ENTER) {
                _client_button_down = false;
            } else if (key <= K_SPACE) {
                return;
            } else if (key >= K_BACKSPACE) {
                return;
            } else if (len < 13) {
                cls.info.name[len+1] = 0;
                cls.info.name[len] = key;
            }

            return;
        }
        else
            _client_button_down = false;
    }
    else if ( _server_button_down )
    {
        if ( _menu_active )
        {
            if ( !down )
                return;

            if ( shift )
                key = _shift_keys[key];

            std::size_t len = strnlen(svs.name, countof(svs.name));

            if (key == K_BACKSPACE && len) {
                svs.name[strlen(svs.name)-1] = 0;
            } else if (key == K_ENTER) {
                _server_button_down = false;
            } else if (key < K_SPACE) {
                return;
            } else if (key >= K_BACKSPACE) {
                return;
            } else if (len < countof(svs.name)) {
                svs.name[len+1] = 0;
                svs.name[len] = key;
            }

            return;
        }
        else
            _server_button_down = false;
    }
    else if ( _client_say )
    {
        if ( true )
        {
            if ( !down )
                return;

            if ( ctrl && key == 'v' ) {
                std::string s = g_Application->clipboard();

                if (s.length()) {
                    strcat(_clientsay, s.c_str());
                    _clientsay[strlen(_clientsay)] = 0;
                }
                return;
            }

            if ( shift )
                key = _shift_keys[key];

            if ( key == K_BACKSPACE )
                _clientsay[strlen(_clientsay)-1] = 0;
            else if ( key == K_ENTER && strlen(_clientsay) )
            {
                if ( !_clientsay[0] )
                    return;

                if ( _clientsay[0] == '/' )
                {
                    char    *command;

                    // command, parse it
                    if ( (command = strstr( _clientsay, "quit" )) )
                        PostQuitMessage( 0 );
                    else if ( (command = strstr( _clientsay, "find" )) )
                    {
                        strncpy( cls.server, command + 5, SHORT_STRING );
                        find_server( false );
                    }
                    else if ( (command = strstr( _clientsay, "disconnect" )) )
                    {
                        stop_client( );
                        stop_server( );
                    }
                    else if ( (command = strstr( _clientsay, "connect" )) )
                    {
                        if ( strlen( command ) > 8 )
                        {
                            strncpy( cls.server, command + 8, SHORT_STRING );
                            find_server( true );
                        }
                        else
                            connect_to_server( -1 );
                    }
                    else if ( (command = strstr( _clientsay, "set" )) )
                    {
                        if ( strlen( command ) > 4 ) {
                            char    cmdbuf[ 256 ];
                            char    *arg;

                            config::variable_base* variable;

                            strncpy( cmdbuf, command + 4, 256 );

                            //  find next arg
                            for( arg = cmdbuf ; *arg ; arg++ ) {
                                if ( *arg == ' ' ) {
                                    *arg++ = '\0';
                                    break;
                                }
                            }

                            if ( !*arg ) {
                                write_message_client( "usage: set [variable] [value]" );
                            } else if ( (variable = _config->find(cmdbuf)) ) {
                                _config->set(cmdbuf, arg);
                                if (variable->flags() & config::flags::server) {
                                    write_message(va("\'%s\' set to \'%s\'", variable->name(), variable->value().c_str()));
                                } else {
                                    write_message_client(va("\'%s\' set to \'%s\'", variable->name(), variable->value().c_str()));
                                }
                            } else {
                                write_message_client( va("unrecognized variable: %s", cmdbuf ) );
                            }
                        } else {
                            write_message_client( "usage: set [variable] [value]" );
                        }

                    }
                    else if ( (command = strstr( _clientsay, "dedicated" )) )
                    {
                        _dedicated = true;

                        stop_client( );
                        start_server( );
                    }
                    else
                        write_message_client( va("unrecognized command: %s", _clientsay+1) );
                }
                else if (svs.active || cls.active)
                {
                    // say it
                    _netchan.write_byte(clc_say);
                    _netchan.write_string(_clientsay);

                    if (svs.active && !svs.local) {
                        if ( _dedicated )
                            write_message( va( "[Server]: %s", _clientsay ) );
                        else {
                            write_message( va( "\\c%02x%02x%02x%s\\cx: %s",
                                (int )(cls.info.color.r * 255),
                                (int )(cls.info.color.g * 255),
                                (int )(cls.info.color.b * 255),
                                cls.info.name.data(), _clientsay ) );
                        }
                    }
                }

                memset( _clientsay, 0, LONG_STRING );

                _client_say = false;
            }
            else if ( key == K_ENTER )
                _client_say = false;
            else if ( key == K_ESCAPE )
            {
                memset( _clientsay, 0, LONG_STRING );

                _client_say = false;
            }
            else if ( key < K_SPACE )
                return;
            else if ( key > K_BACKSPACE )
                return;
            else if ( strlen(_clientsay) < LONG_STRING )
            {
                _clientsay[strlen(_clientsay)+1] = 0;
                _clientsay[strlen(_clientsay)] = key;
            }

            return;
        }
        else
            _client_say = false;
    }
    else if ( key == K_ENTER && down )
    {
        _client_say = true;
        return;
    }
    else if ( key == '/' && down )
    {
        _client_say = true;
        _clientsay[0] = '/';
        return;
    }
    else if ( key == K_PGDN && down )
    {
        float   time = g_Application->time();

        for ( int i=0 ; i<MAX_MESSAGES ; i++ )
            _messages[i].time = time;
    }

    // user commands here

    if ( ! _dedicated )
    {
        for (int ii = 0; ii < MAX_PLAYERS; ++ii) {
            game::tank* player = _world.player(ii);
            if (player && _clients[ii].input.key_event(key, down)) {
                player->update_usercmd(_clients[ii].input.generate());
            }
        }
    }

    if ( down )
    {
        switch ( key )
        {

        case K_F1:
            write_message_client( "" );
            write_message_client( "----- TANKS HELP -----" );
            write_message_client( "note: pressing PGDN will refresh the message log" );
            write_message_client( "" );
            write_message_client( "  Each player commands an entire tank using the keyboard." );
            write_message_client( "The following keys are using in multiplayer mode: " );
            write_message_client( "W - Forward" );
            write_message_client( "S - Backward" );
            write_message_client( "A - Turns tank left" );
            write_message_client( "D - Turns tank right" );
            write_message_client( "J - Turns turret left" );
            write_message_client( "L - Turns turret right" );
            write_message_client( "K - Fire main gun" );
            write_message_client( "  Shots struck in the rear will do full damage (one shot" );
            write_message_client( "kill with no upgrades), the sides will do 1/2 damage, and" );
            write_message_client( "shots to the front will do 1/3 normal damage." );
            write_message_client( "  You can change your nick and the color of your tank in" );
            write_message_client( "the Game Options menu. You can toggle the menu at any" );
            write_message_client( "time by pressing the ESC key." );
            write_message_client( "  Every 10 kills you achieve in multiplayer you will be" );
            write_message_client( "prompted to upgrade your tank, you can see more about" );
            write_message_client( "upgrades by pressing F9" );
            write_message_client( "" );
            break;

        //
        //  upgrades bullshit
        //

        case '1':
            if ( _clients[cls.number].upgrades )
                write_upgrade( 0 );
            break;

        case '2':
            if ( _clients[cls.number].upgrades )
                write_upgrade( 1 );
            break;

        case '3':
            if ( _clients[cls.number].upgrades )
                write_upgrade( 2 );
            break;

        case '4':
            if ( _clients[cls.number].upgrades )
                write_upgrade( 3 );
            break;

        case K_F9:
            write_message_client( "" );
            write_message_client( "---- UPGRADES HELP ----" );
            write_message_client( "  Upgrades are given every ten kills you achieve. The" );
            write_message_client( "categories you can upgrade in are the following: ");
            write_message_client( "1) Damage - weapon damage" );
            write_message_client( "2) Armor - damage absorption" );
            write_message_client( "3) Gunnery - fire rate" );
            write_message_client( "4) Speed - tank speed" );
            write_message_client( "  To upgrade your tank, press the number associated with" );
            write_message_client( "the upgrade when you have upgrades available to you. You" );
            write_message_client( "should note that when you upgrade your tank a penalty" );
            write_message_client( "will be taken from a complementary category. Damage" );
            write_message_client( "goes with Gunnery, and Speed with Armor. However, when" );
            write_message_client( "you upgrade you will see a net increase in your tanks" );
            write_message_client( "performance." );
            write_message_client( "" );
            break;

        default:
            break;
        }
    }

    if ( ! down )
        return;

    // menu commands

    if (key == K_ESCAPE) {
        _menu_active ^= 1;
        return;
    }

    if (key == K_F2)
    {
        byte    msg[ 2 ];

        msg[ 0 ] = svc_restart;
        msg[ 1 ] = 5;

        broadcast( 2, msg );
        
        _restart_time = _frametime + RESTART_TIME;
        return;
    }
}

//------------------------------------------------------------------------------
void session::cursor_event(vec2 position)
{
    vec2i size = g_Application->window()->size();
    _cursor.x = position.x * 640 / size.x;
    _cursor.y = position.y * 480 / size.y;

    if (_menu_active) {
        _menu.cursor_event(_cursor);
    }
}

//------------------------------------------------------------------------------
void session::add_score(int player_index, int score)
{
    player_index = clamp(player_index,0,MAX_PLAYERS);

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
    int width = _renderer->view().size.x;
    int height = _renderer->view().size.y;

    // draw console/chat box

    if (_client_say) {
        _renderer->draw_string("say:", vec2(width/4,height-16), menu::colors[7]);
        _renderer->draw_string(_clientsay, vec2(width/4+32,height-16), menu::colors[7]);
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
        _renderer->draw_string("for help with upgrades press F9", vec2(8,24), menu::colors[7]);
    }

    // count active players and maximum name width

    int panel_width = 96;
    int active_count = 0;

    if (svs.active || cls.active) {
        for (auto const& cl : svs.clients) {
            if (cl.active) {
                int cl_width = _renderer->string_size(cl.info.name.data()).x;
                panel_width = std::max<int>(panel_width, cl_width + 40);
                active_count++;
            }
        }
    } else {
        active_count = 2;
    }

    // draw the score panel

    _renderer->draw_box(
        vec2(panel_width, active_count*12 + 8),
        vec2(width - panel_width/2 - 16, active_count*6 + 20),
        menu::colors[4]);

    _renderer->draw_box(
        vec2(panel_width - 2, active_count*12 + 8 - 2),
        vec2(width - panel_width/2 - 16, active_count*6 + 20),
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

        std::string score = va("%d", _score[sort[ii]]);
        int score_width = _renderer->string_size(score.c_str()).x;

        _renderer->draw_box(vec2(7,7), vec2(width - panel_width - 8, n*12 + 26), color4(svs.clients[sort[ii]].info.color));
        _renderer->draw_string(svs.clients[sort[ii]].info.name.data(), vec2(width - panel_width - 2, n*12 + 30), menu::colors[7]);
        _renderer->draw_string(score.c_str(), vec2(width - score_width - 20, n*12 + 30), menu::colors[7]);

        n++;
    }

    // draw restart timer

    if (_restart_time > _frametime) {
        int nTime = ceil((_restart_time - _frametime));
        _renderer->draw_string(va("Restart in... %i", nTime), vec2(width/2-48,16+13), menu::colors[7]);
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
        int value = _net_bytes[(_framenum - ii) % _net_bytes.size()];
        sum += value;
        max = std::max<float>(max, value);
    }

    float avg = sum / (std::min<int>(_framenum, _net_bytes.size()) - 1);
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

        std::string smax = va("%0.1f kbps", CHAR_BIT * max / (FRAMETIME * 1024.0f));
        std::string savg = va("%0.1f kbps", CHAR_BIT * avg / (FRAMETIME * 1024.0f));

        _renderer->draw_string(smax.c_str(), vec2(638.0f - _renderer->string_size(smax.c_str()).x, ymax), color4(1,1,1,1));
        _renderer->draw_string(savg.c_str(), vec2(638.0f - _renderer->string_size(savg.c_str()).x, yavg), color4(1,1,1,alpha_avg));
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
    _restart_time = 0.0f;
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

        if (!_restart_time || !_world.player(i)) {
            spawn_player(i);
        }
    }

    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::restart()
{
    _restart_time = 0.0f;

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
void session::spawn_player(int num)
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
int session::message(char const* format, ...)
{
    va_list list;
    char    string[MAX_STRING];

    va_start( list, format );
    vsprintf( string, format, list );
    va_end( list );

    strcpy( _messages[_num_messages].string, string );
    _messages[_num_messages].time = g_Application->time();

    _num_messages = (_num_messages+1)%MAX_MESSAGES;

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
void session::write_message (char const* message, bool broadcast)
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
    int         i;
    int         ypos;
    float       alpha;

    float       time = g_Application->time();

    ypos = DEFAULT_H - 36;

    for ( i=_num_messages-1 ; i!=_num_messages ; i = ( i<=0 ? MAX_MESSAGES-1 : i-1 ) )
    {
        if ( i < 0 )
            continue;

        if ( _messages[i].time+15.0f > time )
        {
            alpha = (_messages[i].time+12.0f > time ? 1.0f : (_messages[i].time+15.0f - time)/3.0f );

            _renderer->draw_string(_messages[i].string, vec2(8,ypos), color4(1,1,1,alpha));

            ypos -= 12;
        }
    }
}

//------------------------------------------------------------------------------
static bool gs_try_connect = false;

int session::find_server_by_name(void*)
{
    //g_Game->write_message( va("searching for: %s", g_Game->cls.server ) );

    //if ( !pNet->string_to_address( g_Game->cls.server, &g_Game->_netserver ) )
    //    g_Game->write_message( va("could not find server: %s", g_Game->cls.server ) );
    //else
    //{
    //    g_Game->write_message( va("found: %s", pNet->address_to_string( g_Game->_netserver) ) );
    //    g_Game->_have_server = true;
    //}

    //if ( g_Game->_have_server && gs_try_connect )
    //    g_Game->connect_to_server( -1 );

    return 0;
}

} // namespace game

//------------------------------------------------------------------------------
void find_server(bool connect)
{
    unsigned int    id;

    game::gs_try_connect = connect;

    CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE )g_Game->find_server_by_name, NULL, NULL, (LPDWORD )&id );
}
