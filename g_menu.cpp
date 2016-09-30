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
void menu_func_newgame	() { g_Game->m_StopClient( ); g_Game->m_StopServer( ); g_Game->NewGame( );	}
void menu_func_reset	() { g_Game->Reset( );		}
void menu_func_quit		() { g_Game->m_StopClient( ); g_Game->m_StopServer( ); g_Application->Quit( 0 );	}

void menu_func_host		() { g_Game->m_StartServer( ); }
void menu_func_join		() { g_Game->m_ConnectToServer( ); }
void menu_func_refresh	() { g_Game->m_InfoAsk( ); }

/*
===========================================================

Name	:	cMenu::Init

Purpose	:	Handles GUI

===========================================================
*/

void cMenu::Init ()
{
	// base

	m_SubMenus[0] = new cMenu;	// local
	m_SubMenus[1] = new cMenu;	// network
	AddButton( new cMenuButton(	"Network Game",		vec2(56,32),	vec2(96,32),	this,	m_SubMenus[1] ) );
	AddButton( new cMenuButton( "Local Game",		vec2(56,80),	vec2(96,32),	this,	m_SubMenus[0] ) );
	AddButton( new cMenuButton( "Video Options",	vec2(56,128),	vec2(96,32),	this,	NULL ) );
	AddButton( new cBaseButton( "Quit",				vec2(56,176),	vec2(96,32),	menu_func_quit ) );

	// local

	m_SubMenus[0]->m_SubMenus[0] = new cMenu;	// local->options
	m_SubMenus[0]->AddButton( new cCondButton( "Resume",	vec2(144,32),	vec2(64,32),	menu_func_resume,	&g_Game->m_bGameActive ) );	
	m_SubMenus[0]->AddButton( new cBaseButton( "New Round",	vec2(144,80),	vec2(64,32),	menu_func_newgame ) );
	m_SubMenus[0]->AddButton( new cBaseButton( "Reset",		vec2(144,128),	vec2(64,32),	menu_func_reset ) );
	m_SubMenus[0]->AddButton( new cMenuButton( "Options",	vec2(144,176),	vec2(64,32),	m_SubMenus[0],	m_SubMenus[0]->m_SubMenus[0] ) );

	// local->options

	m_SubMenus[0]->m_SubMenus[0]->m_SubMenus[0] = new cOptionsMenu("Game Options", vec2(440,56), vec2(96,80));	// local->options->options
	m_SubMenus[0]->m_SubMenus[0]->AddButton( new cTankButton( "Player 1", vec2(232,56), vec2(96,80), &g_Game->m_Players[0] ) );
	m_SubMenus[0]->m_SubMenus[0]->AddButton( new cTankButton( "Player 2", vec2(336,56), vec2(96,80), &g_Game->m_Players[1] ) );
	m_SubMenus[0]->m_SubMenus[0]->ActivateMenu( NULL, m_SubMenus[0]->m_SubMenus[0]->m_SubMenus[0] );	// force activate

	// local->options->game_options

	m_SubMenus[0]->m_SubMenus[0]->m_SubMenus[0]->AddButton( new cCheckButton( "Extended Armor", vec2(440-38,38), &g_Game->bExtendedArmor ) );
	m_SubMenus[0]->m_SubMenus[0]->m_SubMenus[0]->AddButton( new cCheckButton( "Random Spawn", vec2(440-38,52), &g_Game->bRandomSpawn ) );
	m_SubMenus[0]->m_SubMenus[0]->m_SubMenus[0]->AddButton( new cCheckButton( "Auto Restart", vec2(440-38,66), &g_Game->bAutoRestart ) );

	// network

	m_SubMenus[1]->AddButton( new cCondButton( "Resume",	vec2(144,32),	vec2(64,32),	menu_func_resume,	&g_Game->m_bGameActive ) );	
	m_SubMenus[1]->AddButton( new cBaseButton( "Host",		vec2(144,80),	vec2(64,32),	menu_func_host	)	);
	m_SubMenus[1]->AddButton( new cCondButton( "Join",		vec2(144,128),	vec2(64,32),	menu_func_join,		&g_Game->m_bHaveServer	)	);
	m_SubMenus[1]->AddButton( new cBaseButton( "Refresh",	vec2(144,176),	vec2(64,32),	menu_func_refresh	)	);
	m_SubMenus[1]->AddButton( new cClientButton( "Player", vec2(232,56), vec2(96,80), &g_Game->cls.color ) );

#if 0
	// options
	m_SubMenus[2]->m_SubMenus[0] = new cMenu;	// options->local
	m_SubMenus[2]->m_SubMenus[1] = new cMenu;	// options->netork
	m_SubMenus[2]->AddButton( new cMenuButton( "Local", vec2(144,80), vec2(64,32),		m_SubMenus[2],	m_SubMenus[2]->m_SubMenus[0] )	);
	m_SubMenus[2]->AddButton( new cMenuButton( "Network", vec2(144,32), vec2(64,32),	m_SubMenus[2],	m_SubMenus[2]->m_SubMenus[1] )	);
#endif
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
	g_Render->DrawBox( vec2(190,254), vec2(320,240), 0, menu_colors[5] );

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

/*
===========================================================

Name	:	cOptionsMenu

Purpose	:	behaves a bit different than cMenu

===========================================================
*/

cOptionsMenu::cOptionsMenu (char *szTitle, vec2 vPos, vec2 vSize)
{
	strncpy( m_szTitle, szTitle, 64 );
	m_vPos = vPos;
	m_vSize = vSize;
}

void cOptionsMenu::Draw (vec2 vCursorPos)
{
	int			i;

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[4] );
	g_Render->DrawBox( vec2(m_vSize.x-2,m_vSize.y-2), m_vPos, 0, menu_colors[5] );
	g_Render->DrawString( m_szTitle, vec2(m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH,m_vPos.y - m_vSize.y / 2 + 12), menu_colors[7] );

	for (i=0 ; i<MAX_BUTTONS ; i++)
		if (m_Buttons[i])
			m_Buttons[i]->Draw( vCursorPos );
}

void cOptionsMenu::Click (vec2 vCursorPos, bool bDown)
{
	int			i = 0;

	for (i=0 ; i<MAX_BUTTONS ; i++)
		if (m_Buttons[i])
			m_Buttons[i]->Click( vCursorPos, bDown );

	if (m_ActiveMenu)
		m_ActiveMenu->Click( vCursorPos, bDown );
}
