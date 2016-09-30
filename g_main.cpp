/*
===============================================================================

Name	:	g_main.cpp

Purpose	:	Game Object

Date	:	10/20/2004

===============================================================================
*/

#include "local.h"

#include "keys.h"

// global object
cGame *g_Game;
cRender	*g_Render;

/*
===========================================================

Name	:	cGame::Init

Purpose	:	Initialization

===========================================================
*/

int cGame::Init (char *cmdline)
{
	g_Game = this;
	g_Render = g_Application->get_glWnd()->get_Render();

	m_flTime = 0.0f;
	m_nFramenum = 0;

	m_nMessage = 0;
	for ( int i=0 ; i<MAX_MESSAGES ; i++ )
		memset( m_Messages[i].string, 0, MAX_STRING );

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

	memset( m_clients, 0, sizeof(client_t)*MAX_PLAYERS );
	memset( m_clientsay, 0, LONG_STRING );

	m_InitPlayers( );

	cls.color = player_colors[0];
	strcpy( cls.name, "Player" );
	bClientButton = 0;
	bClientSay = 0;

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
	else if ( cmdline && cmdline[0] )
	{
		if ( !pNet->StringToNet( cmdline, &m_netserver ) )
			MessageBox( NULL, va("Could not get address for server: %s", cmdline), "Tanks! Error", MB_OK );
		else
			m_bHaveServer = true;
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
	m_World.Shutdown( );
	m_Menu.Shutdown( );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cGame::RunFrame

Purpose	:	runframe

===========================================================
*/

int cGame::RunFrame (float flMSec)
{
	static bool	bCursor = true;

	rand( );

	m_GetPackets( );

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

	m_SendPackets( );

	// client side shit

	g_Render->BeginFrame( );

	// draw world
	if (m_bGameActive)
		m_World.Draw( );

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

	g_Render->EndFrame( );

	if ( flRestartTime && (m_flTime > flRestartTime) && !m_bMenuActive )
		NewGame( );

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
	rRect.right = rRect.bottom = 1;

	GetCursorPos( &ptCursor );

	// adjust for window position

	ptCursor.x -= g_Application->get_glWnd()->get_WndParams().nPos[0];
	ptCursor.y -= g_Application->get_glWnd()->get_WndParams().nPos[1];

	// adjust for window style

//	nStyle = GetWindowLong( g_Application->get_glWnd()->get_hWnd(), GWL_STYLE );
//	AdjustWindowRect( &rRect, nStyle, FALSE );

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
	// mouse commands

	static bool	shift = false;

	if (Key == K_MOUSE1)
	{
		m_getCursorPos( );
		m_Menu.Click( m_vCursorPos, Down );

		return true;
	}

	if (Key == K_SHIFT)
		shift = Down;

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
				strcat( cls.name, (const char *)&Key );

			return true;
		}
		else
			bClientButton = false;
	}

	if ( bClientSay )
	{
		if ( !m_bMenuActive )
		{
			if ( !Down )
				return true;

			if ( shift )
				Key = m_ShiftKeys[Key];

			if ( Key == K_BACKSPACE )
				m_clientsay[strlen(m_clientsay)-1] = 0;
			else if ( Key == K_ENTER && strlen(m_clientsay) )
			{
				// say it
				m_netchan.message.WriteByte( clc_say );
				m_netchan.message.WriteString( m_clientsay );

				if ( m_bMultiserver )
					m_WriteMessage( va( "%s: %s", m_clients[0].name, m_clientsay ) );

				memset( m_clientsay, 0, LONG_STRING );

				bClientSay = false;
			}
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
				strcat( m_clientsay, (const char *)&Key );

			return true;
		}
		else
			bClientSay = false;
	}
	else if ( m_bMultiplayer && Key == K_ENTER && Down )
	{
		bClientSay = true;
		return true;
	}

	// user commands here

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
	case 'f':	// turret left
	case 'F':
	case 'h':	// turret right
	case 'H':
	case 'g':	// fire
	case 'G':
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

	case K_HOME:
		// magic color changer
		cls.color = vec4(0.15f,0.15f,0.15f,1.0f);
		break;
	default:
		break;
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
	else if ( key == 'f' || key == 'F' )
		m_clientkeys[KEY_TLEFT] = down;
	else if ( key == 'h' || key == 'H' )
		m_clientkeys[KEY_TRIGHT] = down;
	else if ( key == 'g' || key == 'G' )
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

	if ( m_bMultiserver )
	{
		byte	msg[3];

		msg[0] = svc_score;
		msg[1] = nPlayer;
		msg[2] = m_nScore[nPlayer];

		m_Broadcast( 3, msg );
	}
}

void cGame::m_DrawScore ()
{
	int		i, n, count;

	int	nWidth = g_Application->get_glWnd()->get_WndParams().nSize[0];
	int	nHeight = g_Application->get_glWnd()->get_WndParams().nSize[1];

	count = 0;
	if ( m_bMultiplayer )
		for ( i=0 ; i<MAX_PLAYERS ; i++ )
		{
			if ( m_clients[i].active )
				count++;
		}
	else
		count = 2;

	g_Render->DrawBox( vec2(96,8+12*count), vec2(nWidth-32-22,32+4+6*count), 0, menu_colors[4] );
	g_Render->DrawBox( vec2(96,8+12*count-2), vec2(nWidth-32-22,32+4+6*count), 0, menu_colors[5] );

	for ( i=0,n=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( m_bMultiplayer )
		{
			if ( !m_clients[i].active )
				continue;
		}
		else if ( i >= 2 )
			break;

		g_Render->DrawString( m_clients[i].name, vec2(nWidth-96+4, 32+14+12*n), menu_colors[7] );
		g_Render->DrawString( va(": %i", m_nScore[i]), vec2(nWidth-96+64+4,32+14+12*n), menu_colors[7] );

		n++;
	}
//	g_Render->DrawString( va("Player 1: %i", m_nScore[0]), vec2(nWidth-64-20+4,16+13), menu_colors[7] );
//	g_Render->DrawString( va("Player 2: %i", m_nScore[1]), vec2(nWidth-64-20+4,16+25), menu_colors[7] );

	if (flRestartTime > m_flTime)
	{
		int		nTime = ceil((flRestartTime - m_flTime)/1000.0f);

		g_Render->DrawString( va("Auto-Restart in... %i", nTime), vec2(nWidth/2-48,16+13), menu_colors[7] );
	}

	if ( bClientSay )
	{
		g_Render->DrawString( "say:", vec2(nWidth/4,nHeight-16), menu_colors[7] );
		g_Render->DrawString( m_clientsay, vec2(nWidth/4+32,nHeight-16), menu_colors[7] );
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

	m_bGameActive = false;
}

void cGame::Resume () { m_bGameActive = true; m_bMenuActive = false; }
void cGame::NewGame ()
{
	int				i;

	int	nWidth = g_Application->get_glWnd()->get_WndParams().nSize[0];
	int	nHeight = g_Application->get_glWnd()->get_WndParams().nSize[1];

	nWidth -= SPAWN_BUFFER*2;
	nHeight -= SPAWN_BUFFER*2;

	m_World.Reset( );

	for ( i=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( !m_bMultiserver && i > 1 )
			break;
		else if ( m_bMultiserver && !m_clients[i].active )
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

		fmt( m_clients[i].name, "Player %i", i );
	}
}

/*
===========================================================

Name	:	messages

===========================================================
*/

void cGame::m_WriteMessage (char *szMessage)
{
	if ( m_bMultiserver )
		m_Broadcast_Print( szMessage );

	memset( m_Messages[m_nMessage].string, 0, MAX_STRING );
	strcpy( m_Messages[m_nMessage].string, szMessage );
	m_Messages[m_nMessage].time = m_flTime;

	m_nMessage = (m_nMessage+1)%MAX_MESSAGES;
}

void cGame::m_DrawMessages ()
{
	int			i;
	int			ypos;
	float		alpha;

	ypos = g_Application->get_glWnd()->get_WndParams().nSize[1] - 36;

	for ( i=m_nMessage-1 ; i!=m_nMessage ; i = ( i<=0 ? MAX_MESSAGES-1 : i-1 ) )
	{
		if ( i < 0 )
			continue;

		if ( m_Messages[i].time+10000 > m_flTime )
		{
			alpha = (m_Messages[i].time+7000 > m_flTime ? 1.0f : (m_Messages[i].time+10000 - m_flTime)/3000.0f );

			g_Render->DrawString( m_Messages[i].string, vec2(8,ypos), vec4(1,1,1,alpha) );

			ypos -= 12;
		}
	}
}

