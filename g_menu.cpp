/*
===============================================================================

Name	:	g_menu.h

Purpose	:	handles menu functions

Date	:	10/20/2004

===============================================================================
*/

#include "local.h"

cRender	*g_Render;

// menu func routines
void menu_func_quit ()
{
	g_Application->Quit( 0 );
}

void menu_func_resume ()
{
	g_Game->Resume( );
}

void menu_func_newgame ()
{
	g_Game->NewGame( );
}

void menu_func_reset ()
{
	g_Game->Reset( );
}

/*
===========================================================

Name	:	cMenuButton

Purpose	:	individual button in the GUI

===========================================================
*/

void cMenuButton::Init (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click, bool *bCond)
{
	strncpy( m_szTitle, szTitle, 64 );
	m_vPos = vPos;
	m_vSize = vSize;
	m_op_click = op_click;
	m_bCond = bCond;

	m_bOver = false;
	m_bClicked = false;
}

bool cMenuButton::m_Over (vec2 vCursorPos)
{
	return ( (clamp(vCursorPos.x,m_vPos.x-m_vSize.x/2,m_vPos.x+m_vSize.x/2) == vCursorPos.x )
		&& (clamp(vCursorPos.y,m_vPos.y-m_vSize.y/2,m_vPos.y+m_vSize.y/2) == vCursorPos.y ) );
}

bool cMenuButton::Click (vec2 vCursorPos, bool bDown)
{
	if ( m_bCond && !*m_bCond )
	{
		m_bOver = false;
		m_bClicked = false;
		return false;
	}

	m_bOver = m_Over( vCursorPos );

	if (m_bOver && bDown)
	{
		m_bClicked = true;
	}
	else if (m_bClicked && m_bOver && !bDown)
	{
		m_bClicked = false;
		m_op_click();
	}
	else if (!bDown)
	{
		m_bClicked = false;
	}

	return false;
}

void cMenuButton::Draw (vec2 vCursorPos)
{
	int		nColorIn, nColorOut, nColorText;
	vec2	vInSize, vTextPos;

	m_bOver = m_Over( vCursorPos );

	if (m_bOver)
		nColorOut = 6;
	else
		nColorOut = 4;

	if (m_bClicked)
		nColorIn = 3;
	else
		nColorIn = 5;

	nColorText = nColorIn + 2;

	if ( m_bCond && !*m_bCond )
	{
		nColorIn = 2;
		nColorOut = 1;
		nColorText = 3;
	}

	vInSize.x = m_vSize.x - 4;
	vInSize.y = m_vSize.y - 4;

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*3;	// len * char_width / 2
	vTextPos.y = m_vPos.y + 4;						// char_height / 2

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
	g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
	g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

/*
===========================================================

Name	:	cMenu::Init

Purpose	:	Handles GUI

===========================================================
*/

void cMenu::Init ()
{
	g_Render = g_Application->get_glWnd()->get_Render();

	m_Buttons[0].Init( "Resume",	vec2(40,	32),	vec2(64,	32),	menu_func_resume,	&g_Game->m_bGameActive );
	m_Buttons[1].Init( "New Round",	vec2(40,	80),	vec2(64,	32),	menu_func_newgame,	NULL );
	m_Buttons[2].Init( "Reset",		vec2(40,	128),	vec2(64,	32),	menu_func_reset,	NULL );
	m_Buttons[3].Init( "Quit",		vec2(40,	176),	vec2(64,	32),	menu_func_quit,		NULL );
}

void cMenu::Shutdown ()
{
}

/*
===========================================================

Name	:	cMenu::Draw

Purpose	:	Draws the menu to the screen

===========================================================
*/

#define COL1	320-96+4
#define	COL2	320-16
#define	COL3	320+48

#define	ROW		240-128+16

void cMenu::Draw (vec2 vCursorPos)
{
	int			i;

	for (i=0 ; i<NUM_BUTTONS ; i++)
		m_Buttons[i].Draw( vCursorPos );

	// show the keys

	g_Render->DrawBox( vec2(192,256), vec2(320,240), 0, menu_colors[4] );
	g_Render->DrawBox( vec2(188,252), vec2(320,240), 0, menu_colors[5] );

	g_Render->DrawString( "Action:", vec2(COL1,240-128+16), menu_colors[7] );
	g_Render->DrawString( "Player 1:", vec2(COL2,240-128+16), menu_colors[7] );
	g_Render->DrawString( "Player 2:", vec2(COL3,240-128+16), menu_colors[7] );

	// actions
	
	g_Render->DrawString( "Move Ahead", vec2(COL1,ROW+32), menu_colors[7] );
	g_Render->DrawString( "Move Back", vec2(COL1,ROW+48), menu_colors[7] );
	g_Render->DrawString( "Turn Left", vec2(COL1,ROW+64), menu_colors[7] );
	g_Render->DrawString( "Turn Right", vec2(COL1,ROW+80), menu_colors[7] );
	g_Render->DrawString( "Turret Left", vec2(COL1,ROW+96), menu_colors[7] );
	g_Render->DrawString( "Turret Right", vec2(COL1,ROW+112), menu_colors[7] );
	g_Render->DrawString( "Fire!!!", vec2(COL1,ROW+128), menu_colors[7] );

	// player 1 keys

	g_Render->DrawString( "w", vec2(COL2,ROW+32), menu_colors[7] );
	g_Render->DrawString( "s", vec2(COL2,ROW+48), menu_colors[7] );
	g_Render->DrawString( "a", vec2(COL2,ROW+64), menu_colors[7] );
	g_Render->DrawString( "d", vec2(COL2,ROW+80), menu_colors[7] );
	g_Render->DrawString( "f", vec2(COL2,ROW+96), menu_colors[7] );
	g_Render->DrawString( "h", vec2(COL2,ROW+112), menu_colors[7] );
	g_Render->DrawString( "g", vec2(COL2,ROW+128), menu_colors[7] );

	// player 2 keys

	g_Render->DrawString( "Up", vec2(COL3,ROW+32), menu_colors[7] );
	g_Render->DrawString( "Down", vec2(COL3,ROW+48), menu_colors[7] );
	g_Render->DrawString( "Left", vec2(COL3,ROW+64), menu_colors[7] );
	g_Render->DrawString( "Right", vec2(COL3,ROW+80), menu_colors[7] );
	g_Render->DrawString( "KP 4", vec2(COL3,ROW+96), menu_colors[7] );
	g_Render->DrawString( "KP 6", vec2(COL3,ROW+112), menu_colors[7] );
	g_Render->DrawString( "KP 5", vec2(COL3,ROW+128), menu_colors[7] );

	g_Render->DrawString( "New Round", vec2(COL1,ROW+160), menu_colors[7] );
	g_Render->DrawString( "F2", vec2(COL2+32,ROW+160), menu_colors[7] );
}

/*
===========================================================

Name	:	cMenu::Click

Purpose	:	Processes a click from the user

===========================================================
*/

void cMenu::Click (vec2 vCursorPos, bool bDown)
{
	int			i = 0;

	for (i=0 ; i<NUM_BUTTONS ; i++)
		m_Buttons[i].Click( vCursorPos, bDown );
}