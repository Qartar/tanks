/*
===============================================================================

Name    :   g_main.cpp

Purpose :   Game Object

Date    :   10/20/2004

===============================================================================
*/

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

cvar_t  *cl_name = NULL;
cvar_t  *cl_color = NULL;

extern cvar_t   *net_master;        //  master server
extern cvar_t   *net_serverName;    //  server name

// global object
vMain   *pMain;
cGame   *g_Game;
render::system* g_Render;

index_s sound_index[256];

void FindServer (bool bConnect);

#define register_sound(i,a)                                 \
    sound_index[i].index = pSound->Register( a );           \
    sound_index[i].name = (char *)mem::alloc(strlen(a)+1);  \
    strcpy( sound_index[i].name, a );                       \
    *(sound_index[i].name + strlen(a)) = 0

cGame::cGame()
    : m_Players{0}
    , bClientButton(false)
    , bServerButton(false)
{
    g_Game = this;
    pMain = this;
}

/*
===========================================================

Name    :   cGame::Init

Purpose :   Initialization

===========================================================
*/

int cGame::Init (char *cmdline)
{
    g_Render = g_Application->get_glWnd()->get_Render();

    _menu_image = g_Render->load_image(MAKEINTRESOURCE(IDB_BITMAP1));

    g_upgrade_frac      = pVariable->Get( "g_upgradeFrac", "0.50", "float", CVAR_ARCHIVE|CVAR_SERVER, "upgrade fraction" );
    g_upgrade_penalty   = pVariable->Get( "g_upgradePenalty", "0.20", "float", CVAR_ARCHIVE|CVAR_SERVER, "upgrade penalty" );
    g_upgrade_min       = pVariable->Get( "g_upgradeMin", "0.20", "float", CVAR_ARCHIVE|CVAR_SERVER, "minimum upgrade fraction" );
    g_upgrades          = pVariable->Get( "g_upgrades", "true", "bool", CVAR_ARCHIVE|CVAR_SERVER, "enables upgrades" );

    g_arenaWidth        = pVariable->Get( "g_arenaWidth", "640", "int", CVAR_ARCHIVE|CVAR_SERVER|CVAR_RESET, "arena width" );
    g_arenaHeight       = pVariable->Get( "g_arenaHeight", "480", "int", CVAR_ARCHIVE|CVAR_SERVER|CVAR_RESET, "arena height" );

    net_master          = pVariable->Get( "net_master", "oedhead.no-ip.org", "string", CVAR_ARCHIVE, "master server hostname" );
    net_serverName      = pVariable->Get( "net_serverName", "Tanks! Server", "string", CVAR_ARCHIVE, "local server name" );

    m_flTime = 0.0f;
    m_nFramenum = 0;

    m_nMessage = 0;
    for ( int i=0 ; i<MAX_MESSAGES ; i++ )
        memset( m_Messages[i].string, 0, MAX_STRING );

    for ( int i=0 ; i<MAX_SERVERS ; i++ )
    {
        cls.servers[i].active = false;
        memset( cls.servers[i].name, 0, 32 );
    }

    m_bMenuActive = true;
    m_bGameActive = false;

    m_bMultiplayer = false;
    m_bMultiserver = false;
    m_bHaveServer = false;
    m_bMultiactive = false;

    m_bDedicated = false;

    bExtendedArmor = true;
    bRandomSpawn = true;
    bAutoRestart = true;
    bManualRestart = false;

    flRestartTime = 0;

    m_nScore[0] = 0;
    m_nScore[1] = 0;

    svs.active = false;

    memset( svs.clients, 0, sizeof(client_t)*MAX_PLAYERS );
    memset( m_clientsay, 0, LONG_STRING );

    svs.max_clients = MAX_PLAYERS;
    strcpy( svs.name, net_serverName->getString( ) );

    cls.number = 0;

    m_InitClient( );

    m_Menu.init( );
    m_World.init( );

    m_netchan.Init( );

    m_netmsg.Init( m_netmsgbuf, MAX_MSGLEN );
    m_netmsg.Clear( );

    pNet->Config( true );

    m_InfoAsk( );

    if ( strstr( cmdline, "dedicated" ) )
    {
        m_bDedicated = true;
        m_StartServer( );
    }

    for (int i=0 ; i<256 ; i++)
        m_ShiftKeys[i] = i;
    for (int i='a' ; i<='z' ; i++)
        m_ShiftKeys[i] = i - 'a' + 'A';
    m_ShiftKeys['1'] = '!';
    m_ShiftKeys['2'] = '@';
    m_ShiftKeys['3'] = '#';
    m_ShiftKeys['4'] = '$';
    m_ShiftKeys['5'] = '%';
    m_ShiftKeys['6'] = '^';
    m_ShiftKeys['7'] = '&';
    m_ShiftKeys['8'] = '*';
    m_ShiftKeys['9'] = '(';
    m_ShiftKeys['0'] = ')';
    m_ShiftKeys['-'] = '_';
    m_ShiftKeys['='] = '+';
    m_ShiftKeys[','] = '<';
    m_ShiftKeys['.'] = '>';
    m_ShiftKeys['/'] = '?';
    m_ShiftKeys[';'] = ':';
    m_ShiftKeys['\''] = '"';
    m_ShiftKeys['['] = '{';
    m_ShiftKeys[']'] = '}';
    m_ShiftKeys['`'] = '~';
    m_ShiftKeys['\\'] = '|';

    memset( sound_index, 0, sizeof(sound_index) );

    register_sound( 0, "ASSETS\\SOUND\\TANK_MOVE.wav" );
    register_sound( 1, "ASSETS\\SOUND\\TANK_IDLE.wav" );
    register_sound( 2, "ASSETS\\SOUND\\TANK_FIRE.wav" );
    register_sound( 3, "ASSETS\\SOUND\\TANK_EXPLODE.wav" );
    register_sound( 4, "ASSETS\\SOUND\\BULLET_EXPLODE.wav" );
    register_sound( 5, "ASSETS\\SOUND\\TURRET_MOVE.wav" );

    m_WriteMessage( "Welcome to Tanks! Press F1 for help." );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cGame::Shutdown

Purpose :   Shutdown

===========================================================
*/

int cGame::Shutdown ()
{
    m_StopClient( );
    m_EndClient( );

    m_World.shutdown( );
    m_Menu.shutdown( );

    for ( int i=0 ; i<NUM_SOUNDS ; i++ )
        mem::free( sound_index[i].name );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cGame::m_InitClient cGame::m_EndClient

Purpose :   Shutdown

===========================================================
*/

const float colorMinFrac = 0.75f;

void cGame::m_InitClient ()
{
    int     nLength;
    float   csum;

    textutils_c text;

    bClientButton = 0;
    bClientSay = 0;

    cl_name = pVariable->Get( "ui_name", "", "string", CVAR_ARCHIVE, "user info: name" );
    cl_color = pVariable->Get( "ui_color", "255 0 0", "string", CVAR_ARCHIVE, "user info: color" );

    if ( strlen( cl_name->getString( )) )
        strcpy( cls.name, cl_name->getString( ) );
    else
        GetUserName( cls.name, (LPDWORD )&nLength );

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

void cGame::m_EndClient ()
{
    cl_name->setString( cls.name );
    cl_color->setString( va("%i %i %i", (int )(cls.color.r*255), (int )(cls.color.g*255), (int )(cls.color.b*255) ) );
}

/*
===========================================================

Name    :   cGame::RunFrame

Purpose :   runframe

===========================================================
*/

int cGame::RunFrame (float flMSec)
{
    float   beg, getp, wframe, sendp, rmenu, rworld, snd, end;
    float   ref, fbeg;

    static bool bCursor = true;
    bool        bFullscreen = g_Application->get_glWnd()->get_WndParams().bFullscreen;

    rand( );

    beg = g_Application->get_time( );

    m_GetPackets( );

    getp = g_Application->get_time( );

    m_flTime += flMSec;

    if (m_flTime > m_nFramenum * FRAMEMSEC && (!m_bMultiactive || m_bMultiserver) )
    {
        m_nFramenum++;

        // run time step on world
        if ( !m_bMenuActive || m_bMultiserver )
        {
            m_World.run_frame( );
            if ( m_bMultiserver )
                m_WriteFrame( );
        }
    }

    wframe = g_Application->get_time( );

    m_SendPackets( );

    sendp = g_Application->get_time( );

    //
    // client side shit
    //

    ref = g_Application->get_time( );

    g_Render->begin_frame();

    fbeg = g_Application->get_time( );

    //  set view center
    int worldWidth, worldHeight;
    int viewWidth, viewHeight;

    float   centerX, centerY;

    worldWidth  = m_World._maxs.x - m_World._mins.x;
    worldHeight = m_World._maxs.y - m_World._mins.y;

    viewWidth   = DEFAULT_W;
    viewHeight  = DEFAULT_H;

    if ( m_bMultiplayer && svs.clients[ cls.number ].active ) {
        float   flLerp = (m_flTime - (m_nFramenum-1) * FRAMEMSEC) / FRAMEMSEC;

        centerX     = m_Players[ cls.number ]->get_position( flLerp ).x;
        centerY     = m_Players[ cls.number ]->get_position( flLerp ).y;
    } else {
        centerX     = worldWidth / 2;
        centerY     = worldHeight / 2;
    }

    if ( ( centerX < viewWidth / 2 ) && ( centerX > worldWidth - ( viewWidth / 2 )) ) {
        centerX = worldWidth / 2;
    } else if ( centerX < viewWidth / 2 ) {
        centerX = viewWidth / 2;
    } else if ( centerX > worldWidth - ( viewWidth / 2 ) ) {
        centerX = worldWidth - ( viewWidth / 2 );
    }
    if ( ( centerY < viewHeight / 2 ) && ( centerY > worldHeight - ( viewHeight / 2 )) ) {
        centerY = worldHeight / 2;
    } else if ( centerY < viewHeight / 2 ) {
        centerY = viewHeight / 2;
    } else if ( centerY > worldHeight - ( viewHeight / 2 ) ) {
        centerY = worldHeight - ( viewHeight / 2 );
    }
    g_Render->set_view_origin(vec2(centerX - viewWidth / 2, centerY - viewHeight / 2));

    // draw world
    if (m_bGameActive)
        m_World.draw( );

    g_Render->set_view_origin(vec2(0,0));

    rworld = g_Application->get_time( );

    // draw menu
    if (m_bMenuActive)
    {
        if ( !bCursor )
        {
            ShowCursor( TRUE );
            bCursor = true;
        }

        m_getCursorPos( );

        g_Render->draw_image(_menu_image, vec2( 0, 0 ), vec2( 640, 480 ), vec4( 1, 1, 1, 1 ) );

        m_Menu.draw( m_vCursorPos );
    }
    else if ( bCursor )
    {
        ShowCursor( FALSE );
        bCursor = false;
    }

    m_DrawScore( );

    m_DrawMessages( );

    rmenu = g_Application->get_time( );

    g_Render->end_frame();

    // update sound

    pSound->Update( );

    snd = g_Application->get_time( );

    if ( flRestartTime && (m_flTime > flRestartTime) && !m_bMenuActive ) {
        Restart( );
    }

    end = g_Application->get_time( );

    return ERROR_NONE;
}

/*
===========================================================

Name    :   cGame::m_getCursorPos

Purpose :   Gets cursor position from windows and translates it

===========================================================
*/

void cGame::m_getCursorPos ()
{
    POINT   ptCursor;

    sWndParam wndParam = g_Application->get_glWnd()->get_WndParams();

    GetCursorPos( &ptCursor );

    // copy to member value

    m_vCursorPos.x = (ptCursor.x - wndParam.nPos[0]) * DEFAULT_W / wndParam.nSize[0];
    m_vCursorPos.y = (ptCursor.y - wndParam.nPos[1]) * DEFAULT_H / wndParam.nSize[1];
}

/*
===========================================================

Name    :   cGame::Key_Event

Purpose :   Processes key events sent from windows

===========================================================
*/

int cGame::Key_Event (unsigned char Key, bool Down)
{
    static bool shift = false;
    static bool ctrl = false;

    // mouse commands

    if (Key == K_MOUSE1)
    {
        m_getCursorPos( );
        m_Menu.click( m_vCursorPos, Down );

        return true;
    }

    if (Key == K_SHIFT)
        shift = Down;
    if (Key == K_CTRL)
        ctrl = Down;

    if ( bClientButton )
    {
        if ( m_bMenuActive )
        {
            if ( !Down )
                return true;

            if ( shift )
                Key = m_ShiftKeys[Key];

            if ( Key == K_BACKSPACE )
                cls.name[strlen(cls.name)-1] = 0;
            else if ( Key == K_ENTER )
                bClientButton = false;
            else if ( Key <= K_SPACE )
                return true;
            else if ( Key > K_BACKSPACE )
                return true;
            else if ( strlen(cls.name) < 13 )
            {
                cls.name[strlen(cls.name)+1] = 0;
                cls.name[strlen(cls.name)] = Key;
            }

            return true;
        }
        else
            bClientButton = false;
    }
    else if ( bServerButton )
    {
        if ( m_bMenuActive )
        {
            if ( !Down )
                return true;

            if ( shift )
                Key = m_ShiftKeys[Key];

            if ( Key == K_BACKSPACE )
                svs.name[strlen(svs.name)-1] = 0;
            else if ( Key == K_ENTER )
                bServerButton = false;
            else if ( Key < K_SPACE )
                return true;
            else if ( Key > K_BACKSPACE )
                return true;
            else if ( strlen(svs.name) < 32 )
            {
                svs.name[strlen(svs.name)+1] = 0;
                svs.name[strlen(svs.name)] = Key;
            }

            return true;
        }
        else
            bServerButton = false;
    }
    else if ( bClientSay )
    {
        if ( true )
        {
            if ( !Down )
                return true;

            if ( ctrl && Key == 'v' )
            {
                char    *clipboard = g_Application->ClipboardData( );

                if ( clipboard )
                {
                    strcat( m_clientsay, clipboard );
                    m_clientsay[strlen(m_clientsay)] = 0;

                    free( clipboard );
                }
                return true;
            }

            if ( shift )
                Key = m_ShiftKeys[Key];

            if ( Key == K_BACKSPACE )
                m_clientsay[strlen(m_clientsay)-1] = 0;
            else if ( Key == K_ENTER && strlen(m_clientsay) )
            {
                if ( !m_clientsay[0] )
                    return true;

                if ( m_clientsay[0] == '/' )
                {
                    char    *command;

                    // command, parse it
                    if ( (command = strstr( m_clientsay, "quit" )) )
                        PostQuitMessage( 0 );
                    else if ( (command = strstr( m_clientsay, "find" )) )
                    {
                        strncpy( cls.server, command + 5, SHORT_STRING );
                        FindServer( false );
                    }
                    else if ( (command = strstr( m_clientsay, "disconnect" )) )
                    {
                        m_StopClient( );
                        m_StopServer( );
                    }
                    else if ( (command = strstr( m_clientsay, "connect" )) )
                    {
                        if ( strlen( command ) > 8 )
                        {
                            strncpy( cls.server, command + 8, SHORT_STRING );
                            FindServer( true );
                        }
                        else
                            m_ConnectToServer( -1 );
                    }
                    else if ( (command = strstr( m_clientsay, "set" )) )
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
                                m_WriteMessageClient( "usage: set [variable] [value]" );
                            } else if ( (cvar = pVariable->Get( cmdbuf )) != 0 ) {
                                cvar->setString( arg );
                                if ( cvar->getFlags( ) & CVAR_SERVER ) {
                                    m_WriteMessage( va("\'%s\' set to \'%s\'", cvar->getName( ), cvar->getString( ) ) );
                                } else {
                                    m_WriteMessageClient( va("\'%s\' set to \'%s\'", cvar->getName( ), cvar->getString( ) ) );
                                }
                            } else {
                                m_WriteMessageClient( va("unrecognized variable: %s", cmdbuf ) );
                            }
                        } else {
                            m_WriteMessageClient( "usage: set [variable] [value]" );
                        }

                    }
                    else if ( (command = strstr( m_clientsay, "dedicated" )) )
                    {
                        m_bDedicated = true;

                        m_StopClient( );
                        m_StartServer( );
                    }
                    else
                        m_WriteMessageClient( va("unrecognized command: %s", m_clientsay+1) );
                }
                else if ( m_bMultiplayer )
                {
                    // say it
                    m_netchan.message.WriteByte( clc_say );
                    m_netchan.message.WriteString( m_clientsay );

                    if ( m_bMultiserver )
                    {
                        if ( m_bDedicated )
                            m_WriteMessage( va( "[Server]: %s", m_clientsay ) );
                        else {
                            m_WriteMessage( va( "\\c%02x%02x%02x%s\\cx: %s",
                                (int )(cls.color.r * 255),
                                (int )(cls.color.g * 255),
                                (int )(cls.color.b * 255),
                                cls.name, m_clientsay ) ); 
                        }
                    }
                }

                memset( m_clientsay, 0, LONG_STRING );

                bClientSay = false;
            }
            else if ( Key == K_ENTER )
                bClientSay = false;
            else if ( Key == K_ESCAPE )
            {
                memset( m_clientsay, 0, LONG_STRING );

                bClientSay = false;
            }
            else if ( Key < K_SPACE )
                return true;
            else if ( Key > K_BACKSPACE )
                return true;
            else if ( strlen(m_clientsay) < LONG_STRING )
            {
                m_clientsay[strlen(m_clientsay)+1] = 0;
                m_clientsay[strlen(m_clientsay)] = Key;
            }

            return true;
        }
        else
            bClientSay = false;
    }
    else if ( Key == K_ENTER && Down )
    {
        bClientSay = true;
        return true;
    }
    else if ( Key == '/' && Down )
    {
        bClientSay = true;
        m_clientsay[0] = '/';
        return true;
    }
    else if ( Key == K_PGDN && Down )
    {
        float   time = g_Application->get_time( );

        for ( int i=0 ; i<MAX_MESSAGES ; i++ )
            m_Messages[i].time = time;
    }

    // user commands here

    if ( ! m_bDedicated )
    {

        switch ( Key )
        {
        case 'w':   // fwd
        case 'W':
        case 's':   // back
        case 'S':
        case 'a':   // left
        case 'A':
        case 'd':   // right
        case 'D':
        case 'j':   // turret left
        case 'J':
        case 'l':   // turret right
        case 'L':
        case 'k':   // fire
        case 'K':
            if ( (!m_bMultiactive || m_bMultiserver) && m_Players[0] )
                m_Players[0]->update_keys( Key, Down );
            else
                m_ClientKeys( Key, Down );
            break;

        case K_UPARROW:     // fwd
        case K_DOWNARROW:   // back
        case K_LEFTARROW:   // left
        case K_RIGHTARROW:  // right
        case '4':           // turret left
        case K_KP_LEFTARROW:
        case '6':           // turret right
        case K_KP_RIGHTARROW:
        case '5':           // fire
        case K_KP_5:
            if ( !m_bMultiactive && m_Players[1] )
                m_Players[1]->update_keys( Key, Down );
            break;

        default:
            break;
        }
    }

    if ( Down )
    {
        switch ( Key )
        {

        case K_F1:
            m_WriteMessageClient( "" );
            m_WriteMessageClient( "----- TANKS HELP -----" );
            m_WriteMessageClient( "note: pressing PGDN will refresh the message log" );
            m_WriteMessageClient( "" );
            m_WriteMessageClient( "  Each player commands an entire tank using the keyboard." );
            m_WriteMessageClient( "The following keys are using in multiplayer mode: " );
            m_WriteMessageClient( "W - Forward" );
            m_WriteMessageClient( "S - Backward" );
            m_WriteMessageClient( "A - Turns tank left" );
            m_WriteMessageClient( "D - Turns tank right" );
            m_WriteMessageClient( "J - Turns turret left" );
            m_WriteMessageClient( "L - Turns turret right" );
            m_WriteMessageClient( "K - Fire main gun" );
            m_WriteMessageClient( "  Shots struck in the rear will do full damage (one shot" );
            m_WriteMessageClient( "kill with no upgrades), the sides will do 1/2 damage, and" );
            m_WriteMessageClient( "shots to the front will do 1/3 normal damage." );
            m_WriteMessageClient( "  You can change your nick and the color of your tank in" );
            m_WriteMessageClient( "the Game Options menu. You can toggle the menu at any" );
            m_WriteMessageClient( "time by pressing the ESC key." );
            m_WriteMessageClient( "  Every 10 kills you achieve in multiplayer you will be" );
            m_WriteMessageClient( "prompted to upgrade your tank, you can see more about" );
            m_WriteMessageClient( "upgrades by pressing F9" );
            m_WriteMessageClient( "" );
            break;

        //
        //  upgrades bullshit
        //

        case '1':
            if ( gameClients[cls.number].upgrades )
                m_WriteUpgrade( 0 );
            break;

        case '2':
            if ( gameClients[cls.number].upgrades )
                m_WriteUpgrade( 1 );
            break;

        case '3':
            if ( gameClients[cls.number].upgrades )
                m_WriteUpgrade( 2 );
            break;

        case '4':
            if ( gameClients[cls.number].upgrades )
                m_WriteUpgrade( 3 );
            break;

        case K_F9:
            m_WriteMessageClient( "" );
            m_WriteMessageClient( "---- UPGRADES HELP ----" );
            m_WriteMessageClient( "  Upgrades are given every ten kills you achieve. The" );
            m_WriteMessageClient( "categories you can upgrade in are the following: ");
            m_WriteMessageClient( "1) Damage - weapon damage" );
            m_WriteMessageClient( "2) Armor - damage absorption" );
            m_WriteMessageClient( "3) Gunnery - fire rate" );
            m_WriteMessageClient( "4) Speed - tank speed" );
            m_WriteMessageClient( "  To upgrade your tank, press the number associated with" );
            m_WriteMessageClient( "the upgrade when you have upgrades available to you. You" );
            m_WriteMessageClient( "should note that when you upgrade your tank a penalty" );
            m_WriteMessageClient( "will be taken from a complementary category. Damage" );
            m_WriteMessageClient( "goes with Gunnery, and Speed with Armor. However, when" );
            m_WriteMessageClient( "you upgrade you will see a net increase in your tanks" );
            m_WriteMessageClient( "performance." );
            m_WriteMessageClient( "" );
            break;

        default:
            break;
        }
    }

    if ( ! Down )
        return false;

    // menu commands

    if (Key == K_ESCAPE)
    {
        if ( ! m_bGameActive )
            return false;

        m_bMenuActive ^= 1;

        return true;
    }

    if (Key == K_F2)
    {
        byte    msg[ 2 ];

        msg[ 0 ] = svc_restart;
        msg[ 1 ] = 5;

        m_Broadcast( 2, msg );
        
        flRestartTime = m_flTime + 5000.0f;
        bManualRestart = true;
        return true;
    }

    return false;
}

void cGame::m_ClientKeys (int key, bool down)
{
    if ( key == 'w' || key == 'W' )
        m_clientkeys[KEY_FORWARD] = down;
    else if ( key == 's' || key == 'S' )
        m_clientkeys[KEY_BACK] = down;
    else if ( key == 'a' || key == 'A' )
        m_clientkeys[KEY_LEFT] = down;
    else if ( key == 'd' || key == 'D' )
        m_clientkeys[KEY_RIGHT] = down;
    else if ( key == 'j' || key == 'J' )
        m_clientkeys[KEY_TLEFT] = down;
    else if ( key == 'l' || key == 'L' )
        m_clientkeys[KEY_TRIGHT] = down;
    else if ( key == 'k' || key == 'K' )
        m_clientkeys[KEY_FIRE] = down;
}

/*
===========================================================

Name    :   Score functions

===========================================================
*/

void cGame::AddScore (int nPlayer, int nScore)
{
    nPlayer = clamp(nPlayer,0,MAX_PLAYERS);

    m_nScore[nPlayer] += nScore;

    if ( m_bMultiplayer ) {
        if ( m_nScore[nPlayer] % 10 == 0 ) {
            if ( g_upgrades->getBool( ) ) {
                gameClients[nPlayer].upgrades++;
            }
        }
    }

    if ( m_bMultiserver )
    {
        netmsg_t    netmsg;
        byte        buf[MAX_MSGLEN];

        byte    msg[3];

        msg[0] = svc_score;
        msg[1] = nPlayer;
        msg[2] = m_nScore[nPlayer];

        m_Broadcast( 3, msg );

        netmsg.Init( buf, MAX_MSGLEN );
        m_WriteInfo( nPlayer, &netmsg );
        m_Broadcast( netmsg.nCurSize, netmsg.pData );
    }
}

void cGame::m_DrawScore ()
{
    int     i, n, count;

    int     sort[ MAX_PLAYERS ];

    int nWidth = DEFAULT_W;
    int nHeight = DEFAULT_H;

    if ( bClientSay )
    {
        g_Render->draw_string("say:", vec2(nWidth/4,nHeight-16), menu::colors[7]);
        g_Render->draw_string(m_clientsay, vec2(nWidth/4+32,nHeight-16), menu::colors[7]);
    }

    if ( m_bMenuActive )
        return;

    count = 0;
    if ( m_bMultiplayer )
        for ( i=0 ; i<MAX_PLAYERS ; i++ )
        {
            if ( svs.clients[i].active )
                count++;
        }
    else
        count = 2;

    if ( gameClients[cls.number].upgrades )
    {
        int num = gameClients[cls.number].upgrades;

        if ( num > 1 )
            g_Render->draw_string(va( "you have %i upgrades waiting...", num ), vec2(8,12), menu::colors[7]);
        else
            g_Render->draw_string("you have 1 upgrade waiting...", vec2(8,12), menu::colors[7]);
        g_Render->draw_string("for help with upgrades press F9", vec2(8,24), menu::colors[7]);
    }

    g_Render->draw_box(vec2(96,8+12*count), vec2(nWidth-32-22,32+4+6*count), menu::colors[4]);
    g_Render->draw_box(vec2(96,8+12*count-2), vec2(nWidth-32-22,32+4+6*count), menu::colors[5]);

    memset( sort, -1, sizeof(sort) );
    for ( i=0 ; i<MAX_PLAYERS ; i++ ) {

        if ( m_bMultiplayer ) {
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
            if ( m_nScore[ sort[ n ] ] >= m_nScore[ i ] ) {
                break;
            }
        }
        sort[ n ] = i;
    }


    for ( i=0,n=0 ; i<MAX_PLAYERS ; i++ )
    {
        if ( m_bMultiplayer )
        {
            if ( !svs.clients[ sort[ i ] ].active )
                continue;
        }
        else if ( i >= 2 )
            break;

        g_Render->draw_box(vec2(7,7), vec2(nWidth-96, 32+11+12*n),
            vec4(m_Players[sort[i]]->_color.r, m_Players[sort[i]]->_color.g, m_Players[sort[i]]->_color.b, 1));

        g_Render->draw_string(svs.clients[ sort[ i ] ].name, vec2(nWidth-96+4, 32+14+12*n), menu::colors[7]);
        g_Render->draw_string(va(": %i", m_nScore[ sort[ i ] ]), vec2(nWidth-96+64+4,32+14+12*n), menu::colors[7]);

        n++;
    }

    if ( flRestartTime > m_flTime )
    {
        int     nTime = ceil((flRestartTime - m_flTime)/1000.0f);

        g_Render->draw_string(va("Restart in... %i", nTime), vec2(nWidth/2-48,16+13), menu::colors[7]);
    }
}

/*
===========================================================

Name    :   Menu functions

===========================================================
*/

void cGame::Reset ()
{
    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        m_nScore[i] = 0;

        gameClients[i].armor_mod = 1.0f;
        gameClients[i].damage_mod = 1.0f;
        gameClients[i].refire_mod = 1.0f;
        gameClients[i].speed_mod = 1.0f;
        gameClients[i].upgrades = 0;

        m_Players[i] = nullptr;
    }

    m_bGameActive = false;
    m_World.reset( );
}

void cGame::Resume () { m_bGameActive = true; m_bMenuActive = false; }
void cGame::NewGame ()
{
    flRestartTime = 0.0f;
    m_World.clear_particles( );

    bManualRestart = false;

    if ( m_bMultiplayer && !m_bMultiserver ) {
        return;
    }

    //
    //  reset world
    //

    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        m_Players[i] = nullptr;
    }

    m_World.reset( );

    //
    //  reset scores
    //

    if ( m_bMultiserver && bManualRestart )
    {
        netmsg_t    netmsg;
        byte        buf[MAX_MSGLEN];

        netmsg.Init( buf, MAX_MSGLEN );
        for ( int i=0 ; i<MAX_PLAYERS ; i++ ) {
            netmsg.WriteByte( svc_score );  //  score command
            netmsg.WriteByte( i );          //  player index
            netmsg.WriteByte( 0 );          //  current score
        }
        m_Broadcast( netmsg.nCurSize, netmsg.pData );
    }

    //
    //  reset players
    //

    for ( int i=0 ; i<MAX_PLAYERS ; i++ )
    {
        gameClients[i].armor_mod = 1.0f;
        gameClients[i].damage_mod = 1.0f;
        gameClients[i].refire_mod = 1.0f;
        gameClients[i].speed_mod = 1.0f;
        gameClients[i].upgrades = 0;

        if ( bManualRestart ) {
            m_nScore[ i ] = 0;
        }

        if ( !m_bMultiserver && i > 1 )
            break;
        else if ( m_bMultiserver && !svs.clients[i].active )
            continue;

        if ( !flRestartTime || !m_Players[i] || m_Players[i]->_damage >= 1.0f)
        {
            spawn_player(i);
        }
    }

    m_bGameActive = true;
    m_bMenuActive = false;
}

void cGame::Restart ()
{
    flRestartTime = 0.0f;
    bManualRestart = false;

    if ( m_bMultiplayer && !m_bMultiserver ) {
        return;
    }

    for (int ii = 0; ii < MAX_PLAYERS; ++ii)
    {
        if ( !m_bMultiserver && ii > 1 )
            break;
        else if ( m_bMultiserver && !svs.clients[ii].active )
            continue;

        if (m_Players[ii]->_damage >= 1.0f)
            respawn_player(ii);
        else
            m_Players[ii]->_damage = 0.0f;
    }

    m_bGameActive = true;
    m_bMenuActive = false;
}

void cGame::spawn_player(int num)
{
    //
    //  initialize tank object
    //

    assert(m_Players[num] == nullptr);
    m_Players[num] = m_World.spawn<game::tank>();

    m_Players[num]->_model = &tank_body_model;
    m_Players[num]->_turret_model = &tank_turret_model;
    m_Players[num]->_color = player_colors[num];
    m_Players[num]->_player_index = num;
    m_Players[num]->_client = gameClients + num;

    respawn_player(num);

    //
    //  initialize stats
    //

    m_nScore[num] = 0;

    gameClients[num].color = player_colors[num];
    gameClients[num].armor_mod = 1.0f;
    gameClients[num].damage_mod = 1.0f;
    gameClients[num].refire_mod = 1.0f;
    gameClients[num].speed_mod = 1.0f;
    gameClients[num].upgrades = 0;

    fmt( svs.clients[num].name, "Player %i", num+1 );
}

void cGame::respawn_player(int num)
{
    int nWidth = m_World._maxs.x - SPAWN_BUFFER * 2;
    int nHeight = m_World._maxs.y - SPAWN_BUFFER * 2;

    assert(m_Players[num] != nullptr);

    m_Players[num]->set_position(vec2(nWidth*frand()+SPAWN_BUFFER,nHeight*frand()+SPAWN_BUFFER));
    m_Players[num]->set_rotation(frand()*2.0f*M_PI);
    m_Players[num]->_turret_rotation = m_Players[num]->get_rotation();

    m_Players[num]->_old_position = m_Players[num]->get_position();
    m_Players[num]->_old_rotation = m_Players[num]->get_rotation();
    m_Players[num]->_old_turret_rotation = m_Players[num]->_turret_rotation;

    m_Players[num]->set_linear_velocity(vec2(0,0));
    m_Players[num]->set_angular_velocity(0.0f);
    m_Players[num]->_turret_velocity = 0.0f;
    m_Players[num]->_track_speed = 0.0f;

    m_Players[num]->_damage = 0.0f;
    m_Players[num]->_fire_time = 0.0f;
}

/*
===========================================================

Name    :   messages

===========================================================
*/

int cGame::Message (char *szMessage, ...)
{
    va_list list;
    char    string[MAX_STRING];

    va_start( list, szMessage );
    vsprintf( string, szMessage, list );
    va_end( list );

    return ERROR_NONE;
}

void cGame::m_WriteMessage (char *szMessage, bool broadcast)
{
    if ( m_bMultiserver && broadcast )
        m_Broadcast_Print( szMessage );

    memset( m_Messages[m_nMessage].string, 0, MAX_STRING );
    strcpy( m_Messages[m_nMessage].string, szMessage );
    m_Messages[m_nMessage].time = g_Application->get_time( );

    m_nMessage = (m_nMessage+1)%MAX_MESSAGES;
}

void cGame::m_DrawMessages ()
{
    int         i;
    int         ypos;
    float       alpha;

    float       time = g_Application->get_time( );

    ypos = DEFAULT_H - 36;

    for ( i=m_nMessage-1 ; i!=m_nMessage ; i = ( i<=0 ? MAX_MESSAGES-1 : i-1 ) )
    {
        if ( i < 0 )
            continue;

        if ( m_Messages[i].time+15000 > time )
        {
            alpha = (m_Messages[i].time+12000 > time ? 1.0f : (m_Messages[i].time+15000 - time)/3000.0f );

            g_Render->draw_string(m_Messages[i].string, vec2(8,ypos), vec4(1,1,1,alpha));

            ypos -= 12;
        }
    }
}

/*
===========================================================

Name    :   FindServer

Purpose :   asynchronous search for a server

            this has a very large potential for fucking something up

===========================================================
*/

static bool gs_bConnect;

int cGame::FindServerByName (void *lpvoid)
{
    g_Game->m_WriteMessage( va("searching for: %s", g_Game->cls.server ) );

    if ( !pNet->StringToNet( g_Game->cls.server, &g_Game->m_netserver ) )
        g_Game->m_WriteMessage( va("could not find server: %s", g_Game->cls.server ) );
    else
    {
        g_Game->m_WriteMessage( va("found: %s", pNet->NetToString( g_Game->m_netserver) ) );
        g_Game->m_bHaveServer = true;
    }

    if ( g_Game->m_bHaveServer && gs_bConnect )
        g_Game->m_ConnectToServer( -1 );

    return 0;
}

void FindServer (bool bConnect)
{
    unsigned int    id;

    gs_bConnect = bConnect;

    CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE )g_Game->FindServerByName, NULL, NULL, (LPDWORD )&id );
}
