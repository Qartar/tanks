/*
===============================================================================

Name	:	g_menu.h

Purpose	:	handles menu functions

Date	:	10/20/2004

===============================================================================
*/

#include "local.h"

// menu func routines
void menu_func_resume	() { g_Game->Resume( );		}
void menu_func_newgame	() { g_Game->NewGame( );	}
void menu_func_reset	() { g_Game->Reset( );		}
void menu_func_quit		() { g_Application->Quit( 0 );	}

/*
===========================================================

Name	:	cMenu::Init

Purpose	:	Handles GUI

===========================================================
*/

void cMenu::Init ()
{
	// base

	m_SubMenus[0] = new cMenu;	// multiplayer
//	m_SubMenus[1] = new cMenu;	// video options
	AddButton( new cMenuButton(	"Network Game",		vec2(56,32),	vec2(96,32),	NULL,	NULL ) );
	AddButton( new cMenuButton( "Local Game",		vec2(56,80),	vec2(96,32),	this,	m_SubMenus[0] ) );
	AddButton( new cMenuButton( "Video Options",	vec2(56,128),	vec2(96,32),	NULL,	NULL ) );
	AddButton( new cBaseButton( "Quit",				vec2(56,176),	vec2(96,32),	menu_func_quit ) );

	// multiplayer

	m_SubMenus[0]->m_SubMenus[0] = new cMenu;	// multiplayer->options
	m_SubMenus[0]->AddButton( new cCondButton( "Resume",	vec2(144,32),	vec2(64,32),	menu_func_resume,	&g_Game->m_bGameActive ) );	
	m_SubMenus[0]->AddButton( new cBaseButton( "New Round",	vec2(144,80),	vec2(64,32),	menu_func_newgame ) );
	m_SubMenus[0]->AddButton( new cBaseButton( "Reset",		vec2(144,128),	vec2(64,32),	menu_func_reset ) );
	m_SubMenus[0]->AddButton( new cMenuButton( "Options",	vec2(144,176),	vec2(64,32),	m_SubMenus[0],	m_SubMenus[0]->m_SubMenus[0] ) );

	// multiplater->options

	m_SubMenus[0]->m_SubMenus[0]->AddButton( new cTankButton( "Player 1", vec2(232,56), vec2(96,80), &g_Game->m_Players[0] ) );
	m_SubMenus[0]->m_SubMenus[0]->AddButton( new cTankButton( "Player 2", vec2(336,56), vec2(96,80), &g_Game->m_Players[1] ) );

	// video options

//	m_SubMenus[0]->

	
}


void cMenu::Shutdown ()
{
	int			i;

	for (i=0 ; i<MAX_BUTTONS ; i++)
		if (m_Buttons[i])
			delete m_Buttons[i];

	for (i=0 ; i<MAX_SUBMENUS ; i++)
		if (m_SubMenus[i])
		{
			m_SubMenus[i]->Shutdown( );
			delete m_SubMenus[i];
		}
}

void cMenu::AddButton (cBaseButton *pButton)
{
	int			i;

	for (i=0 ; i<MAX_BUTTONS ; i++)
	{
		if (m_Buttons[i])
			continue;

		m_Buttons[i] = pButton;
		return;
	}

	// couldn't add it, delete it to avoid mem leaks
	delete pButton;
}

bool cMenu::ActivateMenu (cMenuButton *pButton, cMenu *pActiveMenu)
{
	if (pActiveMenu == m_ActiveMenu)
	{
		m_ActiveMenu = NULL;
		m_ActiveButton = NULL;
		return false;	// deactivates caller
	}

	if (m_ActiveButton)
		m_ActiveButton->Deactivate( );
	m_ActiveButton = pButton;

	m_ActiveMenu = pActiveMenu;

	return true;
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

	// draw buttons

	for (i=0 ; i<MAX_BUTTONS ; i++)
		if (m_Buttons[i])
			m_Buttons[i]->Draw( vCursorPos );

	// draw submenu

	if (m_ActiveMenu)
		m_ActiveMenu->Draw( vCursorPos );

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

	// shortcuts

	g_Render->DrawString( "New Round", vec2(COL1,ROW+160), menu_colors[7] );
	g_Render->DrawString( "F2", vec2(COL2+32,ROW+160), menu_colors[7] );
	g_Render->DrawString( "Menu", vec2(COL1,ROW+176), menu_colors[7] );
	g_Render->DrawString( "Esc", vec2(COL2+32,ROW+176), menu_colors[7] );
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

	for (i=0 ; i<MAX_BUTTONS ; i++)
		if (m_Buttons[i])
			m_Buttons[i]->Click( vCursorPos, bDown );

	if (m_ActiveMenu)
		m_ActiveMenu->Click( vCursorPos, bDown );
}