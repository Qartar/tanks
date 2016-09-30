/*
===============================================================================

Name	:	g_main.cpp

Purpose	:	Game Object

Date	:	10/20/2004

===============================================================================
*/

#include "local.h"

#include "keys.h"

#include "cm_sound.h"
#include "cm_variable.h"

cvar_t	*g_upgrade_frac = NULL;
cvar_t	*g_upgrade_penalty = NULL;
cvar_t	*g_upgrade_min = NULL;

#include <windows.h>
#include <direct.h>
#include <fstream>
using namespace std;

// global object
vMain	*pMain;
cGame	*g_Game;
cRender	*g_Render;

index_s	sound_index[256];

void FindServer (bool bConnect);

#define register_sound(i,a)									\
	sound_index[i].index = pSound->Register( a );			\
	sound_index[i].name = (char *)mem::alloc(strlen(a)+1);	\
	strcpy( sound_index[i].name, a );						\
	*(sound_index[i].name + strlen(a)) = 0

#include "cm_variable.h"

cvar_t	*cl_name;
cvar_t	*cl_color;

/*
===========================================================

Name	:	cGame::Init

Purpose	:	Initialization

===========================================================
*/

int cGame::Init (char *cmdline)
{
	g_Game = this;
	pMain = this;
	g_Render = g_Application->get_glWnd()->get_Render();

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

	flRestartTime = 0;

	m_nScore[0] = 0;
	m_nScore[1] = 0;

	svs.active = false;

	memset( svs.clients, 0, sizeof(client_t)*MAX_PLAYERS );
	memset( m_clientsay, 0, LONG_STRING );

	svs.max_clients = MAX_PLAYERS;
	strcpy( svs.name, "Tanks! Server" );

	cls.number = 0;

	m_InitPlayers( );
	m_InitClient( );

	m_Menu.Init( );
	m_World.Init( );

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

	register_sound( 0, "SOUND/TANK_MOVE.wav" );
	register_sound( 1, "SOUND/TANK_IDLE.wav" );
	register_sound( 2, "SOUND/TANK_FIRE.wav" );
	register_sound( 3, "SOUND/TANK_EXPLODE.wav" );
	register_sound( 4, "SOUND/BULLET_EXPLODE.wav" );
	register_sound( 5, "SOUND/TURRET_MOVE.wav" );

	m_WriteMessage( "Welcome to Tanks! Press F1 for help." );

	g_upgrade_frac = pVariable->Get( "g_upgrade_frac", "0.50", "float", CVAR_ARCHIVE, "upgrade fraction" );
	g_upgrade_penalty = pVariable->Get( "g_upgrade_penalty", "0.20", "float", CVAR_ARCHIVE, "upgrade penalty" );
	g_upgrade_min = pVariable->Get( "g_upgrade_min", "0.20", "float", CVAR_ARCHIVE, "minimum upgrade fraction" );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cGame::Shutdown

Purpose	:	Shutdown

===========================================================
*/

int cGame::Shutdown ()
{
	m_StopClient( );
	m_EndClient( );

	m_World.Shutdown( );
	m_Menu.Shutdown( );

	for ( int i=0 ; i<NUM_SOUNDS ; i++ )
		mem::free( sound_index[i].name );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cGame::m_InitClient cGame::m_EndClient

Purpose	:	Shutdown

===========================================================
*/

#define CLIENT_INI_PATH	"My Documents/My Games/Tanks!"
#define CLIENT_INI_FILE	"My Documents/My Games/Tanks!/Tanks!.cfg"

void cGame::m_InitClient ()
{
	int		nLength;
	char	szPath[LONG_STRING];

	textutils_c	text;

	fstream	fs;

	bClientButton = 0;
	bClientSay = 0;

	nLength = LONG_STRING;
	ExpandEnvironmentStrings( "%homedrive%%homepath%", szPath, (DWORD )nLength );

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
}

void cGame::m_EndClient ()
{
	cl_name->setString( cls.name );
	cl_color->setString( va("%i %i %i", (int )(cls.color.r*255), (int )(cls.color.g*255), (int )(cls.color.b*255) ) );
}

/*
===========================================================

Name	:	cGame::RunFrame

Purpose	:	runframe

===========================================================
*/

int cGame::RunFrame (float flMSec)
{
	float	beg, getp, wframe, sendp, rmenu, rworld, snd, end;
	float	ref, fbeg;

	static bool	bCursor = true;
	bool		bFullscreen = g_Application->get_glWnd()->get_WndParams().bFullscreen;

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
			m_World.RunFrame( );
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

	g_Application->get_glWnd()->Refresh( );

	ref = g_Application->get_time( );

	g_Render->BeginFrame( );

	fbeg = g_Application->get_time( );

	// draw world
	if (m_bGameActive)
		m_World.Draw( );

	rworld = g_Application->get_time( );

	m_DrawMessages( );

	// draw menu
	if (m_bMenuActive)
	{
		if ( !bCursor )
		{
			ShowCursor( TRUE );
			bCursor = true;
		}

		m_getCursorPos( );

		m_Menu.Draw( m_vCursorPos );
	}
	else if ( bCursor )
	{
		ShowCursor( FALSE );
		bCursor = false;
	}

	m_DrawScore( );

	rmenu = g_Application->get_time( );

	g_Render->EndFrame( );

	// update sound

	pSound->Update( );

	snd = g_Application->get_time( );

	if ( flRestartTime && (m_flTime > flRestartTime) && !m_bMenuActive )
		NewGame( );

	end = g_Application->get_time( );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cGame::m_getCursorPos

Purpose	:	Gets cursor position from windows and translates it

===========================================================
*/

void cGame::m_getCursorPos ()
{
	POINT	ptCursor;
	RECT	rRect;

	rRect.left = rRect.top = 0;

	rRect.right = g_Application->get_glWnd()->get_WndParams().nSize[0];
	rRect.bottom = g_Application->get_glWnd()->get_WndParams().nSize[1];

	GetCursorPos( &ptCursor );

	// adjust for window position

	ptCursor.x -= g_Application->get_glWnd()->get_WndParams().nPos[0];
	ptCursor.y -= g_Application->get_glWnd()->get_WndParams().nPos[1];

	// adjust for window style

//	if ( !g_Application->get_glWnd()->get_WndParams().bFullscreen )
//	{
//		nStyle = GetWindowLong( g_Application->get_glWnd()->get_hWnd(), GWL_STYLE );
//		AdjustWindowRect( &rRect, nStyle, FALSE );
//	}

	// copy to member value

	m_vCursorPos.x = ptCursor.x - rRect.left;
	m_vCursorPos.y = ptCursor.y - rRect.top;
}

/*
===========================================================

Name	:	cGame::Key_Event

Purpose	:	Processes key events sent from windows

===========================================================
*/

int cGame::Key_Event (unsigned char Key, bool Down)
{
	static bool	shift = false;
	static bool ctrl = false;

	// mouse commands

	if (Key == K_MOUSE1)
	{
		m_getCursorPos( );
		m_Menu.Click( m_vCursorPos, Down );

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
				char	*clipboard = g_Application->ClipboardData( );

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
					char	*command;

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
					else if ( (command = strstr( m_clientsay, "dedicated" )) )
					{
						m_bDedicated = true;

						m_StopClient( );
						m_StartServer( );
					}
					else
						m_WriteMessage( va("unrecognized command: %s", m_clientsay+1) );
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
//							m_WriteMessage( va( "%s: %s", cls.name, m_clientsay ) );
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
		for ( int i=0 ; i<MAX_MESSAGES ; i++ )
			m_Messages[i].time = m_flTime;
	}

	// user commands here

	if ( ! m_bDedicated )
	{

		switch ( Key )
		{
		case 'w':	// fwd
		case 'W':
		case 's':	// back
		case 'S':
		case 'a':	// left
		case 'A':
		case 'd':	// right
		case 'D':
		case 'j':	// turret left
		case 'J':
		case 'l':	// turret right
		case 'L':
		case 'k':	// fire
		case 'K':
			if ( !m_bMultiactive || m_bMultiserver )
				m_Players[0].UpdateKeys( Key, Down );
			else
				m_ClientKeys( Key, Down );
			break;

		case K_UPARROW:		// fwd
		case K_DOWNARROW:	// back
		case K_LEFTARROW:	// left
		case K_RIGHTARROW:	// right
		case '4':			// turret left
		case K_KP_LEFTARROW:
		case '6':			// turret right
		case K_KP_RIGHTARROW:
		case '5':			// fire
		case K_KP_5:
			if ( !m_bMultiactive )
				m_Players[1].UpdateKeys( Key, Down );
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
			m_WriteMessage( "" );
			m_WriteMessage( "----- TANKS HELP -----" );
			m_WriteMessage( "note: pressing PGDN will refresh the message log" );
			m_WriteMessage( "" );
			m_WriteMessage( "  Each player commands an entire tank using the keyboard." );
			m_WriteMessage( "The following keys are using in multiplayer mode: " );
			m_WriteMessage( "W - Forward" );
			m_WriteMessage( "S - Backward" );
			m_WriteMessage( "A - Turns tank left" );
			m_WriteMessage( "D - Turns tank right" );
			m_WriteMessage( "J - Turns turret left" );
			m_WriteMessage( "L - Turns turret right" );
			m_WriteMessage( "K - Fire main gun" );
			m_WriteMessage( "  Shot struck in the rear will do full damage (one shot" );
			m_WriteMessage( "kill with no upgrades), the sides will do 1/2 damage, and" );
			m_WriteMessage( "shots to the front will do 1/3 normal damage." );
			m_WriteMessage( "  You can change your nick and the color of your tank in" );
			m_WriteMessage( "the Game Options menu. You can toggle the menu at any" );
			m_WriteMessage( "time by pressing the ESC key." );
			m_WriteMessage( "  Every 10 kills you achieve in multiplayer you will be" );
			m_WriteMessage( "prompted to upgrade your tank, you can see more about" );
			m_WriteMessage( "upgrades by pressing F9" );
			m_WriteMessage( "" );
			break;

		//
		//	upgrades bullshit
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
			m_WriteMessage( "" );
			m_WriteMessage( "---- UPGRADES HELP ----" );
			m_WriteMessage( "  Upgrades are given every ten kills you achieve. The" );
			m_WriteMessage( "categories you can upgrade in are the following: ");
			m_WriteMessage( "1) Damage - weapon damage" );
			m_WriteMessage( "2) Armor - damage absorption" );
			m_WriteMessage( "3) Gunnery - fire rate" );
			m_WriteMessage( "4) Speed - tank speed" );
			m_WriteMessage( "  To upgrade your tank, press the number associated with" );
			m_WriteMessage( "the upgrade when you have upgrades available to you. You" );
			m_WriteMessage( "should note than when you upgrade your tank, a penalty" );
			m_WriteMessage( "will be taken from a corresponding category. Damage" );
			m_WriteMessage( "goes with Gunnery, and Speed with Armor. However, when" );
			m_WriteMessage( "you upgrade you will see a net increase in your tanks" );
			m_WriteMessage( "performance." );
			m_WriteMessage( "" );
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
		NewGame ();
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

Name	:	Score functions

===========================================================
*/

void cGame::AddScore (int nPlayer, int nScore)
{
	nPlayer = clamp(nPlayer,0,MAX_PLAYERS);

	m_nScore[nPlayer] += nScore;

	if ( m_nScore[nPlayer] % 10 == 0 )
		gameClients[nPlayer].upgrades++;

	if ( m_bMultiserver )
	{
		netmsg_t	netmsg;
		byte		buf[MAX_MSGLEN];

		byte	msg[3];

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
	int		i, n, count;

	int	nWidth = g_Application->get_glWnd()->get_WndParams().nSize[0];
	int	nHeight = g_Application->get_glWnd()->get_WndParams().nSize[1];

	if ( bClientSay )
	{
		g_Render->DrawString( "say:", vec2(nWidth/4,nHeight-16), menu_colors[7] );
		g_Render->DrawString( m_clientsay, vec2(nWidth/4+32,nHeight-16), menu_colors[7] );
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
		int	num = gameClients[cls.number].upgrades;

		if ( num > 1 )
			g_Render->DrawString( va( "you have %i upgrades waiting...", num ), vec2(8,12), menu_colors[7] );
		else
			g_Render->DrawString( "you have 1 upgrade waiting...", vec2(8,12), menu_colors[7] );
		g_Render->DrawString( "for help with upgrades press F9", vec2(8,24), menu_colors[7] );
	}

	g_Render->DrawBox( vec2(96,8+12*count), vec2(nWidth-32-22,32+4+6*count), 0, menu_colors[4] );
	g_Render->DrawBox( vec2(96,8+12*count-2), vec2(nWidth-32-22,32+4+6*count), 0, menu_colors[5] );

	for ( i=0,n=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( m_bMultiplayer )
		{
			if ( !svs.clients[i].active )
				continue;
		}
		else if ( i >= 2 )
			break;

							//m_WriteMessage( va( "\\c%02x%02x%02x%s\\cx: %s",
							//	(int )(cls.color.r * 255),
							//	(int )(cls.color.g * 255),
							//	(int )(cls.color.b * 255),

		g_Render->DrawBox( vec2(7,7), vec2(nWidth-96, 32+11+12*n), 0,
			vec4( m_Players[i].vColor.r, m_Players[i].vColor.g, m_Players[i].vColor.b, 1 ) );

		//g_Render->DrawString( va( "\\c%02x%02x%02x[]\\cx%s",
		//	(int )(m_Players[i].vColor.r * 255),
		//	(int )(m_Players[i].vColor.g * 255),
		//	(int )(m_Players[i].vColor.b * 255),
		//	svs.clients[i].name ), vec2(nWidth-96+4, 32+14+12*n), menu_colors[7] );

		g_Render->DrawString( svs.clients[i].name, vec2(nWidth-96+4, 32+14+12*n), menu_colors[7] );
		g_Render->DrawString( va(": %i", m_nScore[i]), vec2(nWidth-96+64+4,32+14+12*n), menu_colors[7] );

		n++;
	}

	if (flRestartTime > m_flTime)
	{
		int		nTime = ceil((flRestartTime - m_flTime)/1000.0f);

		g_Render->DrawString( va("Auto-Restart in... %i", nTime), vec2(nWidth/2-48,16+13), menu_colors[7] );
	}
}

/*
===========================================================

Name	:	Menu functions

===========================================================
*/

void cGame::Reset ()
{
	for ( int i=0 ; i<MAX_PLAYERS ; i++ )
		m_nScore[i] = 0;

	for ( int i=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( m_Players[i].channels[0] )
		{
			m_Players[i].channels[0]->stopSound( );
			m_Players[i].channels[1]->stopSound( );
			m_Players[i].channels[2]->stopSound( );
		}
	}

	m_bGameActive = false;
	m_World.Reset( );
}

void cGame::Resume () { m_bGameActive = true; m_bMenuActive = false; }
void cGame::NewGame ()
{
	int				i;

	int	nWidth = 640;
	int	nHeight = 480;

	nWidth -= SPAWN_BUFFER*2;
	nHeight -= SPAWN_BUFFER*2;

	m_World.Reset( );

	for ( i=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( !m_bMultiserver && i > 1 )
			break;
		else if ( m_bMultiserver && !svs.clients[i].active )
			continue;

		if (bRandomSpawn)
		{
			if ( !flRestartTime || m_Players[i].flDamage >= 1.0f)
			{
				m_Players[i].vPos = vec2(nWidth*frand()+SPAWN_BUFFER,nHeight*frand()+SPAWN_BUFFER);
				m_Players[i].flAngle = frand()*360;
				m_Players[i].flTAngle = m_Players[i].flAngle;

				m_Players[i].vVel = vec2(0,0);
				m_Players[i].flAVel = 0;
				m_Players[i].flTVel = 0;

				m_Players[i].flDamage = 0;
			}
		}
		else
		{
			m_Players[i].vPos = vec2(64,64);
			m_Players[i].flAngle = 0;
			m_Players[i].flTAngle = 0;

			m_Players[i].vVel = vec2(0,0);
			m_Players[i].flAVel = 0;
			m_Players[i].flTVel = 0;

		}

		if ( !m_bMultiserver )
			m_Players[i].flDamage = 0;

		m_World.AddObject( &m_Players[i] );
	}

	m_bGameActive = true;
	m_bMenuActive = false;
	flRestartTime = 0;
}

/*
===========================================================

Name	:	cMain::m_InitPlayers

Purpose	:	Initialized default models and colors

===========================================================
*/

void cGame::m_InitPlayers ()
{
	int				i;

	for ( i=0 ; i<MAX_PLAYERS ; i++ )
	{
		m_Players[i].pModel = &tank_body_model;
		m_Players[i].pTurret = &tank_turret_model;
		m_Players[i].vColor = player_colors[i];
		m_Players[i].nPlayerNum = i;

		m_Players[i].flDeadTime = 0.0f;

		m_Players[i].m_Bullet.nPlayer = i;
		m_Players[i].m_Bullet.pModel = NULL;

		if ( m_Players[i].channels[0] )
		{
			m_Players[i].channels[0]->stopSound( );
			m_Players[i].channels[1]->stopSound( );
			m_Players[i].channels[2]->stopSound( );
		}

		gameClients[i].color = player_colors[i];
		gameClients[i].armor_mod = 1.0f;
		gameClients[i].damage_mod = 1.0f;
		gameClients[i].refire_mod = 1.0f;
		gameClients[i].speed_mod = 1.0f;
		gameClients[i].upgrades = 0;

		m_Players[i].client = gameClients + i;

		fmt( svs.clients[i].name, "Player %i", i );
	}
#if 0
	m_Players[0].pModel = &t80_body_model;
	m_Players[0].pTurret = &t80_turret_model;
	m_Players[1].pModel = &abrams_body_model;
	m_Players[1].pTurret = &abrams_turret_model;
	m_Players[1].pModel = &blkegl_body_model;
	m_Players[1].pTurret = &blkegl_turret_model;

#endif // 0
}

/*
===========================================================

Name	:	messages

===========================================================
*/

int cGame::Message (char *szMessage, ...)
{
	va_list	list;
	char	string[MAX_STRING];

	va_start( list, szMessage );
	vsprintf( string, szMessage, list );
	va_end( list );

//	m_WriteMessage( string );

	return ERROR_NONE;
}

void cGame::m_WriteMessage (char *szMessage)
{
	if ( m_bMultiserver )
		m_Broadcast_Print( szMessage );

	memset( m_Messages[m_nMessage].string, 0, MAX_STRING );
	strcpy( m_Messages[m_nMessage].string, szMessage );
	m_Messages[m_nMessage].time = g_Application->get_time( );

	m_nMessage = (m_nMessage+1)%MAX_MESSAGES;
}

void cGame::m_DrawMessages ()
{
	int			i;
	int			ypos;
	float		alpha;

	float		time = g_Application->get_time( );

	ypos = g_Application->get_glWnd()->get_WndParams().nSize[1] - 36;

	for ( i=m_nMessage-1 ; i!=m_nMessage ; i = ( i<=0 ? MAX_MESSAGES-1 : i-1 ) )
	{
		if ( i < 0 )
			continue;

		if ( m_Messages[i].time+15000 > time )
		{
			alpha = (m_Messages[i].time+12000 > time ? 1.0f : (m_Messages[i].time+15000 - time)/3000.0f );

			g_Render->DrawString( m_Messages[i].string, vec2(8,ypos), vec4(1,1,1,alpha) );

			ypos -= 12;
		}
	}
}

/*
===========================================================

Name	:	FindServer

Purpose	:	asynchronous search for a server

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
	unsigned int	id;

	gs_bConnect = bConnect;

	CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE )g_Game->FindServerByName, NULL, NULL, (LPDWORD )&id );
}
