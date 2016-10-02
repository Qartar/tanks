/*
===============================================================================

Name    :   g_menu.h

Purpose :   handles menu functions

Date    :   10/20/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

// menu func routines
void menu_func_resume   () { g_Game->Resume( );     }
void menu_func_newgame  () { g_Game->m_StopClient( ); g_Game->m_StopServer( ); g_Game->NewGame( );  }
void menu_func_reset    () { g_Game->Reset( );      }
void menu_func_quit     () { g_Game->m_StopClient( ); g_Game->m_StopServer( ); g_Application->Quit( 0 );    }

void menu_func_host     () { g_Game->m_bDedicated = false; g_Game->m_StartServer( ); }
void menu_func_refresh  () { g_Game->m_InfoAsk( ); }

void menu_func_join0    () { g_Game->m_ConnectToServer( 0 ); }
void menu_func_join1    () { g_Game->m_ConnectToServer( 1 ); }
void menu_func_join2    () { g_Game->m_ConnectToServer( 2 ); }
void menu_func_join3    () { g_Game->m_ConnectToServer( 3 ); }

/*
===========================================================

Name    :   cMenu::Init

Purpose :   Handles GUI

===========================================================
*/

void cMenu::Init ()
{
    // base

    m_SubMenus[0] = new cMenu;  // local
    m_SubMenus[1] = new cMenu;  // network
    m_SubMenus[2] = new cMenu;  // options
    AddButton( new cCondButton( "Resume",           vec2(64,32),    vec2(96,32),    menu_func_resume,   &g_Game->m_bGameActive ) );
    AddButton( new cMenuButton( "Network Game",     vec2(192,32),   vec2(96,32),    this,   m_SubMenus[1] ) );
    AddButton( new cMenuButton( "Local Game",       vec2(320,32),   vec2(96,32),    this,   m_SubMenus[0] ) );
    AddButton( new cMenuButton( "Game Options",     vec2(448,32),   vec2(96,32),    this,   m_SubMenus[2] ) );
    AddButton( new cBaseButton( "Quit",             vec2(576,32),   vec2(96,32),    menu_func_quit ) );

    // local

    m_SubMenus[0]->AddButton( new cBaseButton( "New Round", vec2(48,80),    vec2(64,32),    menu_func_newgame ) );
    m_SubMenus[0]->AddButton( new cBaseButton( "Reset",     vec2(48,128),   vec2(64,32),    menu_func_reset ) );

    // network

    m_SubMenus[1]->m_SubMenus[0] = new cMenu;
    m_SubMenus[1]->m_SubMenus[1] = new cMenu;
    m_SubMenus[1]->AddButton( new cMenuButton( "Host",      vec2(48,80),    vec2(64,24),    m_SubMenus[1], m_SubMenus[1]->m_SubMenus[0] ) );
    m_SubMenus[1]->AddButton( new cMenuButton( "Join",      vec2(128,80),   vec2(64,24),    m_SubMenus[1], m_SubMenus[1]->m_SubMenus[1] ) );

    // network->host

    m_SubMenus[1]->m_SubMenus[0]->AddButton( new cHostButton( vec2(144,128), vec2(256,24),  menu_func_host ) );

    // network->join

    m_SubMenus[1]->m_SubMenus[1]->AddButton( new cBaseButton( "Refresh",    vec2(240,80),   vec2(64,24),    menu_func_refresh   )   );
    m_SubMenus[1]->m_SubMenus[1]->AddButton( new cServerButton( vec2(144,128), vec2(256,24), g_Game->cls.servers[0].name, &g_Game->cls.servers[0].ping, menu_func_join0 ) );
    m_SubMenus[1]->m_SubMenus[1]->AddButton( new cServerButton( vec2(144,160), vec2(256,24), g_Game->cls.servers[1].name, &g_Game->cls.servers[1].ping, menu_func_join1 ) );
    m_SubMenus[1]->m_SubMenus[1]->AddButton( new cServerButton( vec2(144,192), vec2(256,24), g_Game->cls.servers[2].name, &g_Game->cls.servers[2].ping, menu_func_join2 ) );
    m_SubMenus[1]->m_SubMenus[1]->AddButton( new cServerButton( vec2(144,224), vec2(256,24), g_Game->cls.servers[3].name, &g_Game->cls.servers[3].ping, menu_func_join3 ) );

    // options

    m_SubMenus[2]->AddButton( new cClientButton( "Player",  vec2(64,104),   vec2(96,80),    &g_Game->cls.color ) );
}

void cMenu::Shutdown ()
{
    int         i;

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
    int         i;

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
        return false;   // deactivates caller
    }

    if (m_ActiveButton)
        m_ActiveButton->Deactivate( );
    m_ActiveButton = pButton;

    m_ActiveMenu = pActiveMenu;

    return true;
}


/*
===========================================================

Name    :   cMenu::Draw

Purpose :   Draws the menu to the screen

===========================================================
*/

#define COL1    320-96+4
#define COL2    320-16
#define COL3    320+48

#define ROW     240-128+16

void cMenu::Draw (vec2 vCursorPos)
{
    int         i;

    // draw buttons

    for (i=0 ; i<MAX_BUTTONS ; i++)
        if (m_Buttons[i])
            m_Buttons[i]->Draw( vCursorPos );

    // draw submenu

    if (m_ActiveMenu)
        m_ActiveMenu->Draw( vCursorPos );
}

/*
===========================================================

Name    :   cMenu::Click

Purpose :   Processes a click from the user

===========================================================
*/

void cMenu::Click (vec2 vCursorPos, bool bDown)
{
    int         i = 0;

    for (i=0 ; i<MAX_BUTTONS ; i++)
        if (m_Buttons[i])
            m_Buttons[i]->Click( vCursorPos, bDown );

    if (m_ActiveMenu)
        m_ActiveMenu->Click( vCursorPos, bDown );
}

/*
===========================================================

Name    :   cOptionsMenu

Purpose :   behaves a bit different than cMenu

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
    int         i;

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[4] );
    g_Render->DrawBox( vec2(m_vSize.x-2,m_vSize.y-2), m_vPos, 0, menu_colors[5] );
    g_Render->DrawString( m_szTitle, vec2(m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH,m_vPos.y - m_vSize.y / 2 + 12), menu_colors[7] );

    for (i=0 ; i<MAX_BUTTONS ; i++)
        if (m_Buttons[i])
            m_Buttons[i]->Draw( vCursorPos );
}

void cOptionsMenu::Click (vec2 vCursorPos, bool bDown)
{
    int         i = 0;

    for (i=0 ; i<MAX_BUTTONS ; i++)
        if (m_Buttons[i])
            m_Buttons[i]->Click( vCursorPos, bDown );

    if (m_ActiveMenu)
        m_ActiveMenu->Click( vCursorPos, bDown );
}
