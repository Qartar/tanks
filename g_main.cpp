// g_main.cpp
//

#include "local.h"
#pragma hdrstop

#include "keys.h"
#include "resource.h"

cvar_t  *g_upgrade_frac = NULL;
cvar_t  *g_upgrade_penalty = NULL;
cvar_t  *g_upgrade_min = NULL;
cvar_t  *g_upgrades = NULL;

cvar_t  *g_arenaWidth = NULL;
cvar_t  *g_arenaHeight = NULL;

extern cvar_t   *net_master;        //  master server
extern cvar_t   *net_serverName;    //  server name

// global object
vMain   *pMain;
game::session* g_Game;

void find_server(bool connect);

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
session::session()
    : _players{0}
    , _client_button_down(false)
    , _server_button_down(false)
{
    g_Game = this;
    pMain = this;
}

//------------------------------------------------------------------------------
int session::init (char *cmdline)
{
    _renderer = g_Application->window()->renderer();

    _menu_image = _renderer->load_image(MAKEINTRESOURCE(IDB_BITMAP1));

    g_upgrade_frac      = pVariable->Get( "g_upgradeFrac", "0.50", "float", CVAR_ARCHIVE|CVAR_SERVER, "upgrade fraction" );
    g_upgrade_penalty   = pVariable->Get( "g_upgradePenalty", "0.20", "float", CVAR_ARCHIVE|CVAR_SERVER, "upgrade penalty" );
    g_upgrade_min       = pVariable->Get( "g_upgradeMin", "0.20", "float", CVAR_ARCHIVE|CVAR_SERVER, "minimum upgrade fraction" );
    g_upgrades          = pVariable->Get( "g_upgrades", "true", "bool", CVAR_ARCHIVE|CVAR_SERVER, "enables upgrades" );

    g_arenaWidth        = pVariable->Get( "g_arenaWidth", "640", "int", CVAR_ARCHIVE|CVAR_SERVER|CVAR_RESET, "arena width" );
    g_arenaHeight       = pVariable->Get( "g_arenaHeight", "480", "int", CVAR_ARCHIVE|CVAR_SERVER|CVAR_RESET, "arena height" );

    net_master          = pVariable->Get( "net_master", "oedhead.no-ip.org", "string", CVAR_ARCHIVE, "master server hostname" );
    net_serverName      = pVariable->Get( "net_serverName", "Tanks! Server", "string", CVAR_ARCHIVE, "local server name" );

    _frametime = 0.0f;
    _framenum = 0;

    _num_messages = 0;
    for ( int i=0 ; i<MAX_MESSAGES ; i++ )
        memset( _messages[i].string, 0, MAX_STRING );

    for ( int i=0 ; i<MAX_SERVERS ; i++ )
    {
        cls.servers[i].active = false;
        memset( cls.servers[i].name, 0, 32 );
    }

    _menu_active = true;
    _game_active = false;

    _multiplayer = false;
    _multiserver = false;
    _have_server = false;
    _multiplayer_active = false;

    _dedicated = false;

    _extended_armor = true;
    _random_spawn = true;

    _restart_time = 0;

    _score[0] = 0;
    _score[1] = 0;

    svs.active = false;

    memset( svs.clients, 0, sizeof(client_t)*MAX_PLAYERS );
    memset( _clientsay, 0, LONG_STRING );

    svs.max_clients = MAX_PLAYERS;
    strcpy( svs.name, net_serverName->getString( ) );

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

    _netchan.init( );

    _netmsg.init( _netmsgbuf, MAX_MSGLEN );
    _netmsg.clear( );

    pNet->config( true );

    info_ask( );

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
    pSound->load_sound("assets/sound/tank_fire.wav");
    pSound->load_sound("assets/sound/tank_explode.wav");
    pSound->load_sound("assets/sound/bullet_explode.wav");
    pSound->load_sound("assets/sound/turret_move.wav");

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

    if (_frametime > _framenum * FRAMEMSEC && (!_multiplayer_active || _multiserver) )
    {
        _framenum++;

        // run time step on world
        if ( !_menu_active || _multiserver )
        {
            _world.run_frame( );
            if ( _multiserver )
                write_frame( );
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
    static bool show_cursor = true;

    _renderer->begin_frame();

    //  set view center
    int world_width, world_height;
    int view_width, view_height;

    float   center_x, center_y;

    world_width  = _world.maxs().x - _world.mins().x;
    world_height = _world.maxs().y - _world.mins().y;

    view_width   = DEFAULT_W;
    view_height  = DEFAULT_H;

    if ( _multiplayer && svs.clients[ cls.number ].active ) {
        float lerp = (_frametime - (_framenum-1) * FRAMEMSEC) / FRAMEMSEC;

        center_x = _players[ cls.number ]->get_position( lerp ).x;
        center_y = _players[ cls.number ]->get_position( lerp ).y;
    } else {
        center_x = world_width / 2;
        center_y = world_height / 2;
    }

    if ( ( center_x < view_width / 2 ) && ( center_x > world_width - ( view_width / 2 )) ) {
        center_x = world_width / 2;
    } else if ( center_x < view_width / 2 ) {
        center_x = view_width / 2;
    } else if ( center_x > world_width - ( view_width / 2 ) ) {
        center_x = world_width - ( view_width / 2 );
    }
    if ( ( center_y < view_height / 2 ) && ( center_y > world_height - ( view_height / 2 )) ) {
        center_y = world_height / 2;
    } else if ( center_y < view_height / 2 ) {
        center_y = view_height / 2;
    } else if ( center_y > world_height - ( view_height / 2 ) ) {
        center_y = world_height - ( view_height / 2 );
    }
    _renderer->set_view_origin(vec2(center_x - view_width / 2, center_y - view_height / 2));

    // draw world
    if (_game_active)
        _world.draw(_renderer);

    _renderer->set_view_origin(vec2(0,0));

    // draw menu
    if (_menu_active)
    {
        if ( !show_cursor )
        {
            ShowCursor( TRUE );
            show_cursor = true;
        }

        get_cursor( );

        _renderer->draw_image(_menu_image, vec2( 0, 0 ), vec2( 640, 480 ), vec4( 1, 1, 1, 1 ) );

        _menu.draw(_renderer, _cursor);
    }
    else if ( show_cursor )
    {
        ShowCursor( FALSE );
        show_cursor = false;
    }

    draw_score( );

    draw_messages( );

    _renderer->end_frame();
}

//------------------------------------------------------------------------------
void session::get_cursor ()
{
    vec2 position = g_Application->window()->position();
    vec2 size = g_Application->window()->size();

    POINT pt; GetCursorPos(&pt);

    // copy to member value

    _cursor.x = (pt.x - position.x) * DEFAULT_W / size.x;
    _cursor.y = (pt.y - position.y) * DEFAULT_H / size.y;
}

//------------------------------------------------------------------------------
int session::key_event(unsigned char key, bool down)
{
    static bool shift = false;
    static bool ctrl = false;

    // mouse commands

    if (key == K_MOUSE1)
    {
        get_cursor( );
        _menu.click( _cursor, down );

        return true;
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
                return true;

            if ( shift )
                key = _shift_keys[key];

            if ( key == K_BACKSPACE )
                cls.name[strlen(cls.name)-1] = 0;
            else if ( key == K_ENTER )
                _client_button_down = false;
            else if ( key <= K_SPACE )
                return true;
            else if ( key > K_BACKSPACE )
                return true;
            else if ( strlen(cls.name) < 13 )
            {
                cls.name[strlen(cls.name)+1] = 0;
                cls.name[strlen(cls.name)] = key;
            }

            return true;
        }
        else
            _client_button_down = false;
    }
    else if ( _server_button_down )
    {
        if ( _menu_active )
        {
            if ( !down )
                return true;

            if ( shift )
                key = _shift_keys[key];

            if ( key == K_BACKSPACE )
                svs.name[strlen(svs.name)-1] = 0;
            else if ( key == K_ENTER )
                _server_button_down = false;
            else if ( key < K_SPACE )
                return true;
            else if ( key > K_BACKSPACE )
                return true;
            else if ( strlen(svs.name) < 32 )
            {
                svs.name[strlen(svs.name)+1] = 0;
                svs.name[strlen(svs.name)] = key;
            }

            return true;
        }
        else
            _server_button_down = false;
    }
    else if ( _client_say )
    {
        if ( true )
        {
            if ( !down )
                return true;

            if ( ctrl && key == 'v' )
            {
                char    *clipboard = g_Application->ClipboardData( );

                if ( clipboard )
                {
                    strcat( _clientsay, clipboard );
                    _clientsay[strlen(_clientsay)] = 0;

                    free( clipboard );
                }
                return true;
            }

            if ( shift )
                key = _shift_keys[key];

            if ( key == K_BACKSPACE )
                _clientsay[strlen(_clientsay)-1] = 0;
            else if ( key == K_ENTER && strlen(_clientsay) )
            {
                if ( !_clientsay[0] )
                    return true;

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
                            cvar_t  *cvar;

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
                            } else if ( (cvar = pVariable->Get( cmdbuf )) != 0 ) {
                                cvar->setString( arg );
                                if ( cvar->getFlags( ) & CVAR_SERVER ) {
                                    write_message( va("\'%s\' set to \'%s\'", cvar->getName( ), cvar->getString( ) ) );
                                } else {
                                    write_message_client( va("\'%s\' set to \'%s\'", cvar->getName( ), cvar->getString( ) ) );
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
                else if ( _multiplayer )
                {
                    // say it
                    _netchan.message.write_byte( clc_say );
                    _netchan.message.write_string( _clientsay );

                    if ( _multiserver )
                    {
                        if ( _dedicated )
                            write_message( va( "[Server]: %s", _clientsay ) );
                        else {
                            write_message( va( "\\c%02x%02x%02x%s\\cx: %s",
                                (int )(cls.color.r * 255),
                                (int )(cls.color.g * 255),
                                (int )(cls.color.b * 255),
                                cls.name, _clientsay ) ); 
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
                return true;
            else if ( key > K_BACKSPACE )
                return true;
            else if ( strlen(_clientsay) < LONG_STRING )
            {
                _clientsay[strlen(_clientsay)+1] = 0;
                _clientsay[strlen(_clientsay)] = key;
            }

            return true;
        }
        else
            _client_say = false;
    }
    else if ( key == K_ENTER && down )
    {
        _client_say = true;
        return true;
    }
    else if ( key == '/' && down )
    {
        _client_say = true;
        _clientsay[0] = '/';
        return true;
    }
    else if ( key == K_PGDN && down )
    {
        float   time = g_Application->get_time( );

        for ( int i=0 ; i<MAX_MESSAGES ; i++ )
            _messages[i].time = time;
    }

    // user commands here

    if ( ! _dedicated )
    {
        for (int ii = 0; ii < MAX_PLAYERS; ++ii) {
            if (_players[ii] && _clients[ii].input.key_event(key, down)) {
                _players[ii]->update_usercmd(_clients[ii].input.generate());
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
        return false;

    // menu commands

    if (key == K_ESCAPE)
    {
        if ( ! _game_active )
            return false;

        _menu_active ^= 1;

        return true;
    }

    if (key == K_F2)
    {
        byte    msg[ 2 ];

        msg[ 0 ] = svc_restart;
        msg[ 1 ] = 5;

        broadcast( 2, msg );
        
        _restart_time = _frametime + 5000.0f;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
void session::add_score(int player_index, int score)
{
    player_index = clamp(player_index,0,MAX_PLAYERS);

    _score[player_index] += score;

    if ( _multiplayer ) {
        if ( _score[player_index] % 10 == 0 ) {
            if ( g_upgrades->getBool( ) ) {
                _clients[player_index].upgrades++;
            }
        }
    }

    if ( _multiserver )
    {
        network::message    netmsg;
        byte        buf[MAX_MSGLEN];

        byte    msg[3];

        msg[0] = svc_score;
        msg[1] = player_index;
        msg[2] = _score[player_index];

        broadcast( 3, msg );

        netmsg.init( buf, MAX_MSGLEN );
        write_info( player_index, &netmsg );
        broadcast( netmsg.bytes_written, netmsg.data );
    }
}

//------------------------------------------------------------------------------
void session::draw_score ()
{
    int i, n, count;

    int sort[ MAX_PLAYERS ];

    int width = DEFAULT_W;
    int height = DEFAULT_H;

    if ( _client_say )
    {
        _renderer->draw_string("say:", vec2(width/4,height-16), menu::colors[7]);
        _renderer->draw_string(_clientsay, vec2(width/4+32,height-16), menu::colors[7]);
    }

    if ( _menu_active )
        return;

    count = 0;
    if ( _multiplayer )
        for ( i=0 ; i<MAX_PLAYERS ; i++ )
        {
            if ( svs.clients[i].active )
                count++;
        }
    else
        count = 2;

    if ( _clients[cls.number].upgrades )
    {
        int num = _clients[cls.number].upgrades;

        if ( num > 1 )
            _renderer->draw_string(va( "you have %i upgrades waiting...", num ), vec2(8,12), menu::colors[7]);
        else
            _renderer->draw_string("you have 1 upgrade waiting...", vec2(8,12), menu::colors[7]);
        _renderer->draw_string("for help with upgrades press F9", vec2(8,24), menu::colors[7]);
    }

    _renderer->draw_box(vec2(96,8+12*count), vec2(width-32-22,32+4+6*count), menu::colors[4]);
    _renderer->draw_box(vec2(96,8+12*count-2), vec2(width-32-22,32+4+6*count), menu::colors[5]);

    memset( sort, -1, sizeof(sort) );
    for ( i=0 ; i<MAX_PLAYERS ; i++ ) {

        if ( _multiplayer ) {
            if ( !svs.clients[ i ].active ) {
                continue;
            }
        } else if ( i >= 2 ) {
            break;
        }

        for( n=MAX_PLAYERS-1 ; n>0 ; n-- ) {

            if ( sort[ n-1 ] < 0 ) {
                continue;
            }
            sort[ n ] = sort[ n-1 ];
            if ( _score[ sort[ n ] ] >= _score[ i ] ) {
                break;
            }
        }
        sort[ n ] = i;
    }


    for ( i=0,n=0 ; i<MAX_PLAYERS ; i++ )
    {
        if ( _multiplayer )
        {
            if ( !svs.clients[ sort[ i ] ].active )
                continue;
        }
        else if ( i >= 2 )
            break;

        _renderer->draw_box(vec2(7,7), vec2(width-96, 32+11+12*n),
            vec4(_players[sort[i]]->_color.r, _players[sort[i]]->_color.g, _players[sort[i]]->_color.b, 1));

        _renderer->draw_string(svs.clients[ sort[ i ] ].name, vec2(width-96+4, 32+14+12*n), menu::colors[7]);
        _renderer->draw_string(va(": %i", _score[ sort[ i ] ]), vec2(width-96+64+4,32+14+12*n), menu::colors[7]);

        n++;
    }

    if ( _restart_time > _frametime )
    {
        int     nTime = ceil((_restart_time - _frametime)/1000.0f);

        _renderer->draw_string(va("Restart in... %i", nTime), vec2(width/2-48,16+13), menu::colors[7]);
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

        _players[i] = nullptr;
    }

    _game_active = false;
    _world.reset( );
}

//------------------------------------------------------------------------------
void session::resume()
{
    _game_active = true;
    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::new_game()
{
    _restart_time = 0.0f;
    _world.clear_particles( );

    if ( _multiplayer && !_multiserver ) {
        return;
    }

    //
    //  reset world
    //

    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        _players[i] = nullptr;
    }

    _world.reset( );

    //
    //  reset scores
    //

    if ( _multiserver )
    {
        network::message    netmsg;
        byte        buf[MAX_MSGLEN];

        netmsg.init( buf, MAX_MSGLEN );
        for ( int i=0 ; i<MAX_PLAYERS ; i++ ) {
            netmsg.write_byte( svc_score );  //  score command
            netmsg.write_byte( i );          //  player index
            netmsg.write_byte( 0 );          //  current score
        }
        broadcast( netmsg.bytes_written, netmsg.data );
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

        if ( !_multiserver && i > 1 )
            break;
        else if ( _multiserver && !svs.clients[i].active )
            continue;

        if ( !_restart_time || !_players[i] || _players[i]->_damage >= 1.0f)
        {
            spawn_player(i);
        }
    }

    _game_active = true;
    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::restart()
{
    _restart_time = 0.0f;

    if ( _multiplayer && !_multiserver ) {
        return;
    }

    for (int ii = 0; ii < MAX_PLAYERS; ++ii)
    {
        if ( !_multiserver && ii > 1 )
            break;
        else if ( _multiserver && !svs.clients[ii].active )
            continue;

        if (_players[ii]->_damage >= 1.0f)
            respawn_player(ii);
        else
            _players[ii]->_damage = 0.0f;
    }

    _game_active = true;
    _menu_active = false;
}

//------------------------------------------------------------------------------
void session::spawn_player(int num)
{
    //
    //  initialize tank object
    //

    assert(_players[num] == nullptr);
    _players[num] = _world.spawn<game::tank>();

    _players[num]->_model = &tank_body_model;
    _players[num]->_turret_model = &tank_turret_model;
    _players[num]->_color = player_colors[num];
    _players[num]->_player_index = num;
    _players[num]->_client = _clients + num;

    respawn_player(num);

    //
    //  initialize stats
    //

    _score[num] = 0;

    _clients[num].color = player_colors[num];
    _clients[num].armor_mod = 1.0f;
    _clients[num].damage_mod = 1.0f;
    _clients[num].refire_mod = 1.0f;
    _clients[num].speed_mod = 1.0f;
    _clients[num].upgrades = 0;

    fmt( svs.clients[num].name, "Player %i", num+1 );
}

//------------------------------------------------------------------------------
void session::respawn_player(int num)
{
    int width = _world.maxs().x - _world.mins().x - SPAWN_BUFFER * 2;
    int height = _world.maxs().y - _world.mins().y - SPAWN_BUFFER * 2;

    assert(_players[num] != nullptr);

    _players[num]->set_position(vec2(width*frand()+SPAWN_BUFFER,height*frand()+SPAWN_BUFFER));
    _players[num]->set_rotation(frand()*2.0f*M_PI);
    _players[num]->_turret_rotation = _players[num]->get_rotation();

    _players[num]->_old_position = _players[num]->get_position();
    _players[num]->_old_rotation = _players[num]->get_rotation();
    _players[num]->_old_turret_rotation = _players[num]->_turret_rotation;

    _players[num]->set_linear_velocity(vec2(0,0));
    _players[num]->set_angular_velocity(0.0f);
    _players[num]->_turret_velocity = 0.0f;
    _players[num]->_track_speed = 0.0f;

    _players[num]->_damage = 0.0f;
    _players[num]->_fire_time = 0.0f;
}

//------------------------------------------------------------------------------
int session::message(char const* format, ...)
{
    va_list list;
    char    string[MAX_STRING];

    va_start( list, format );
    vsprintf( string, format, list );
    va_end( list );

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
void session::write_message (char const* message, bool broadcast)
{
    if ( _multiserver && broadcast )
        broadcast_print( message );

    memset( _messages[_num_messages].string, 0, MAX_STRING );
    strcpy( _messages[_num_messages].string, message );
    _messages[_num_messages].time = g_Application->get_time( );

    _num_messages = (_num_messages+1)%MAX_MESSAGES;
}

//------------------------------------------------------------------------------
void session::draw_messages ()
{
    int         i;
    int         ypos;
    float       alpha;

    float       time = g_Application->get_time( );

    ypos = DEFAULT_H - 36;

    for ( i=_num_messages-1 ; i!=_num_messages ; i = ( i<=0 ? MAX_MESSAGES-1 : i-1 ) )
    {
        if ( i < 0 )
            continue;

        if ( _messages[i].time+15000 > time )
        {
            alpha = (_messages[i].time+12000 > time ? 1.0f : (_messages[i].time+15000 - time)/3000.0f );

            _renderer->draw_string(_messages[i].string, vec2(8,ypos), vec4(1,1,1,alpha));

            ypos -= 12;
        }
    }
}

//------------------------------------------------------------------------------
static bool gs_try_connect = false;

int session::find_server_by_name(void *lpvoid)
{
    g_Game->write_message( va("searching for: %s", g_Game->cls.server ) );

    if ( !pNet->string_to_address( g_Game->cls.server, &g_Game->_netserver ) )
        g_Game->write_message( va("could not find server: %s", g_Game->cls.server ) );
    else
    {
        g_Game->write_message( va("found: %s", pNet->address_to_string( g_Game->_netserver) ) );
        g_Game->_have_server = true;
    }

    if ( g_Game->_have_server && gs_try_connect )
        g_Game->connect_to_server( -1 );

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
