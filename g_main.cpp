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

/*
===========================================================

Name	:	cGame::Init

Purpose	:	Initialization

===========================================================
*/

int cGame::Init ()
{
	g_Game = this;

	m_flTime = 0.0f;
	m_nFramenum = 0;

	m_bMenuActive = true;
	m_bGameActive = false;

	m_nScore[0] = 0;
	m_nScore[1] = 0;

	m_Menu.Init( );
	m_World.Init( );

	m_pRender = g_Application->get_glWnd()->get_Render();

	m_World.AddObject( &m_Players[0] );
	m_World.AddObject( &m_Players[1] );

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
	m_flTime += flMSec;
	if (m_flTime > m_nFramenum * 100)
	{
		m_nFramenum++;

		// run time step on world
		if ( ! m_bMenuActive)
			m_World.RunFrame( );
	}

	m_pRender->BeginFrame( );

	// draw world
	if (m_bGameActive)
		m_World.Draw( );

	// draw menu
	if (m_bMenuActive)
	{
		m_getCursorPos( );

		m_Menu.Draw( m_vCursorPos );
	}

	m_DrawScore( );

	m_pRender->EndFrame( );

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
	int		nStyle;

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

	if (Key == K_MOUSE1)
	{
		m_getCursorPos( );
		m_Menu.Click( m_vCursorPos, Down );

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
		m_Players[0].UpdateKeys( Key, Down );
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
		m_Players[1].UpdateKeys( Key, Down );
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


/*
===========================================================

Name	:	Score functions

===========================================================
*/

void cGame::AddScore (int nPlayer, int nScore)
{
	nPlayer = clamp(nPlayer,0,MAX_PLAYERS);

	m_nScore[nPlayer] += nScore;
}

void cGame::m_DrawScore ()
{
	m_pRender->DrawBox( vec2(64,32), vec2(640-32-22,16+16), 0, menu_colors[4] );
	m_pRender->DrawBox( vec2(60,28), vec2(640-32-22,16+16), 0, menu_colors[5] );

	m_pRender->DrawString( va("Player 1: %i", m_nScore[0]), vec2(640-64-20+4,16+13), menu_colors[7] );
	m_pRender->DrawString( va("Player 2: %i", m_nScore[1]), vec2(640-64-20+4,16+25), menu_colors[7] );
}

/*
===========================================================

Name	:	Menu functions

===========================================================
*/

void cGame::Reset ()
{
	m_nScore[0] = 0;
	m_nScore[1] = 0;

	m_bGameActive = false;
}

void cGame::Resume () { m_bGameActive = true; m_bMenuActive = false; }
void cGame::NewGame ()
{
	m_World.Reset( );
	m_bGameActive = true;
	m_bMenuActive = false;

	m_Players[0].pModel = &tank_body_model;
	m_Players[0].pTurret = &tank_turret_model;
	m_Players[0].vColor = vec4(1,0,0,1);
	m_Players[0].vPos = vec2(64,64);
	m_Players[0].vVel = vec2(0,0);
	m_Players[0].flAVel = 0;
	m_Players[0].flTVel = 0;
	m_Players[0].flAngle = 0;
	m_Players[0].flTAngle = 0;
	m_Players[0].flDamage = 0;
	m_Players[0].nPlayerNum = 0;

	m_Players[1].pModel = &tank_body_model;
	m_Players[1].pTurret = &tank_turret_model;
	m_Players[1].vColor = vec4(0,0,1,1);
	m_Players[1].vPos = vec2(640-64,480-64);
	m_Players[1].vVel = vec2(0,0);
	m_Players[1].flAVel = 0;
	m_Players[1].flTVel = 0;
	m_Players[1].flAngle = 180;
	m_Players[1].flTAngle = 180;
	m_Players[1].flDamage = 0;
	m_Players[1].nPlayerNum = 1;
}