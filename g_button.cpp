/*
===============================================================================

Name    :   g_button.cpp

Purpose :   all the button crap for the menus

Date    :   10/29/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

/*
===========================================================

Name    :   cBaseButton

Purpose :   Basic Base Button class, 

            title
            position
            size
            click operation

===========================================================
*/

cBaseButton::cBaseButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_vSize = vSize;
    m_op_click = op_click;

    m_bOver = false;
    m_bClicked = false;
}

bool cBaseButton::Click (vec2 vCursorPos, bool bDown)
{
    m_bOver = m_Over( vCursorPos );

    if (m_bOver && bDown)
    {
        m_bClicked = true;
    }
    else if (m_bClicked && m_bOver && !bDown)
    {
        m_bClicked = false;
        if (m_op_click) m_op_click();
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cBaseButton::Draw (vec2 vCursorPos)
{
    int     nColorIn, nColorOut, nColorText;
    vec2    vInSize, vTextPos;

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

    vInSize.x = m_vSize.x - 2;
    vInSize.y = m_vSize.y - 2;

    vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;   // len * char_width / 2
    vTextPos.y = m_vPos.y + 4;                              // char_height / 2

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
    g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
    g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

bool cBaseButton::m_Over (vec2 vCursorPos)
{
    return ( (clamp(vCursorPos.x,m_vPos.x-m_vSize.x/2,m_vPos.x+m_vSize.x/2) == vCursorPos.x )
        && (clamp(vCursorPos.y,m_vPos.y-m_vSize.y/2,m_vPos.y+m_vSize.y/2) == vCursorPos.y ) );
}

/*
===========================================================

Name    :   mCondButton

Purpose :   Derived button class

            title
            position
            size
            click operation
            conditional bool

===========================================================
*/

cCondButton::cCondButton (char *szTitle, vec2 vPos, vec2 vSize, func_t op_click, bool *bCond)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_vSize = vSize;
    m_op_click = op_click;
    m_bCond = bCond;

    m_bOver = false;
    m_bClicked = false;
}

bool cCondButton::Click (vec2 vCursorPos, bool bDown)
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
        if (m_op_click) m_op_click();
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cCondButton::Draw (vec2 vCursorPos)
{
    int     nColorIn, nColorOut, nColorText;
    vec2    vInSize, vTextPos;

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

    vInSize.x = m_vSize.x - 2;
    vInSize.y = m_vSize.y - 2;

    vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;   // len * char_width / 2
    vTextPos.y = m_vPos.y + 4;                              // char_height / 2

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
    g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
    g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

/*
===========================================================

Name    :   cMenuButton

Purpose :   Derived button class, activates a given submenu

            title
            position
            size
            parent menu
            submenu

===========================================================
*/

cMenuButton::cMenuButton (char *szTitle, vec2 vPos, vec2 vSize, cMenu *pParent, cMenu *pMenu)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_vSize = vSize;

    m_pParent = pParent;
    m_pMenu = pMenu;

    m_bOver = false;
    m_bClicked = false;
    m_bActive = false;
}

bool cMenuButton::Click (vec2 vCursorPos, bool bDown)
{
    if ( ! m_pMenu )
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
        m_bActive = m_pParent->ActivateMenu( this, m_pMenu );
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cMenuButton::Draw (vec2 vCursorPos)
{
    int     nColorIn, nColorOut, nColorText;
    vec2    vInSize, vTextPos;

    m_bOver = m_Over( vCursorPos );

    if (m_bOver)
        nColorOut = 6;
    else
        nColorOut = 4;

    if (m_bClicked || m_bActive)
        nColorIn = 3;
    else
        nColorIn = 5;

    nColorText = nColorIn + 2;

    if ( ! m_pMenu )
    {
        nColorIn = 2;
        nColorOut = 1;
        nColorText = 3;
    }

    vInSize.x = m_vSize.x - 2;
    vInSize.y = m_vSize.y - 2;

    vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;   // len * char_width / 2
    vTextPos.y = m_vPos.y + 4;                              // char_height / 2

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
    g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
    g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

/*
===========================================================

Name    :   cTankButton

Purpose :   various modifications on a tank

===========================================================
*/

cTankButton::cTankButton (char *szTitle, vec2 vPos, vec2 vSize, cTank *pTank)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_vSize = vSize;

    m_pTank = pTank;
    m_nTankColor = pTank->nPlayerNum;

    m_bOver = false;
    m_bClicked = false;
}

bool cTankButton::Click (vec2 vCursorPos, bool bDown)
{
    m_bOver = m_Over( vCursorPos );

    if (m_bOver && bDown)
    {
        m_bClicked = true;
    }
    else if (m_bClicked && m_bOver && !bDown)
    {
        m_bClicked = false;
        m_nTankColor = (m_nTankColor+1)%NUM_PLAYER_COLORS;
        m_pTank->vColor = player_colors[m_nTankColor];
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cTankButton::Draw (vec2 vCursorPos)
{
    int     nColorIn, nColorOut, nColorText;
    vec2    vInSize, vTextPos;

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

    vInSize.x = m_vSize.x - 2;
    vInSize.y = m_vSize.y - 2;

    vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;   // len * char_width / 2
    vTextPos.y = m_vPos.y + m_vSize.y / 2 - 4;

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
    g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
    g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );

    g_Render->DrawModel( m_pTank->pModel, m_vPos, 0, m_pTank->vColor );
    g_Render->DrawModel( m_pTank->pTurret, m_vPos, 0, m_pTank->vColor );
}

/*
===========================================================

Name    :   cColorButton

Purpose :   set the client color

===========================================================
*/

cClientButton::cClientButton (char *szTitle, vec2 vPos, vec2 vSize, vec4 *pColor)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_vSize = vSize;

    m_pColor = pColor;
    m_nColor = 0;

    m_bOver = false;
    m_bClicked = false;

    m_vTextBoxPos.x = m_vPos.x;
    m_vTextBoxPos.y = m_vPos.y + m_vSize.y / 2 - 12;

    m_vTextBoxSize.x = m_vSize.x - 16;
    m_vTextBoxSize.y = 14;

}

bool cClientButton::Click (vec2 vCursorPos, bool bDown)
{
    m_bOver = m_Over( vCursorPos );

    m_bTextOver = ( (clamp(vCursorPos.x,m_vTextBoxPos.x-m_vTextBoxSize.x/2,m_vPos.x+m_vTextBoxSize.x/2) == vCursorPos.x )
        && (clamp(vCursorPos.y,m_vTextBoxPos.y-m_vTextBoxSize.y/2,m_vTextBoxPos.y+m_vTextBoxSize.y/2) == vCursorPos.y ) );

    if ( m_bTextOver )
    {
        m_bOver = false;
        m_bClicked = false;

        if ( bDown )
        {
            g_Game->bClientButton ^= 1;
            m_bTextDown ^= 1;
        }
    
        return false;
    }
    else
    {
        m_bTextDown = false;
        m_bTextOver = false;
    }

    if (m_bOver && bDown)
    {
        m_bClicked = true;
    }
    else if (m_bClicked && m_bOver && !bDown)
    {
        m_bClicked = false;
        m_nColor = (m_nColor+1)%NUM_PLAYER_COLORS;
        *m_pColor = player_colors[m_nColor];
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }
    return false;
}

void cClientButton::Draw (vec2 vCursorPos)
{
    int     nColorIn, nColorOut, nColorText;
    int     nTextIn, nTextOut;
    vec2    vInSize, vTextPos;

    vec2    vTextBoxText;

    m_bOver = m_Over( vCursorPos );

    m_bTextOver = ( (clamp(vCursorPos.x,m_vTextBoxPos.x-m_vTextBoxSize.x/2,m_vPos.x+m_vTextBoxSize.x/2) == vCursorPos.x )
        && (clamp(vCursorPos.y,m_vTextBoxPos.y-m_vTextBoxSize.y/2,m_vTextBoxPos.y+m_vTextBoxSize.y/2) == vCursorPos.y ) );

    if ( !g_Game->bClientButton )
    {
        m_bTextOver = false;
        m_bTextDown = false;
    }
    else if ( m_bTextOver )
    {
        m_bOver = false;
        m_bClicked = false;
    }
    else
        m_bTextOver = false;

    if ( m_bTextOver )
        nTextOut = 6;
    else
        nTextOut = 4;

    if ( m_bTextDown )
        nTextIn = 5;
    else
        nTextIn = 3;

    if (m_bOver)
        nColorOut = 6;
    else
        nColorOut = 4;

    if (m_bClicked)
        nColorIn = 3;
    else
        nColorIn = 5;

    nColorText = nColorIn + 2;

    vInSize.x = m_vSize.x - 2;
    vInSize.y = m_vSize.y - 2;

    vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;   // len * char_width / 2
    vTextPos.y = m_vPos.y - m_vSize.y / 2 + 16;
    
    vTextBoxText.x = m_vPos.x - m_vSize.x / 2 + 12;
    vTextBoxText.y = m_vTextBoxPos.y + 4;

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
    g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
    g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );

    g_Render->DrawBox( m_vTextBoxSize, m_vTextBoxPos, 0, menu_colors[nTextOut] );
    g_Render->DrawBox( vec2(m_vTextBoxSize.x-2,m_vTextBoxSize.y-2), m_vTextBoxPos, 0, menu_colors[nTextIn] );
    g_Render->DrawString( g_Game->cls.name, vTextBoxText, menu_colors[7] );

    g_Render->DrawModel( &tank_body_model, m_vPos, 0, *m_pColor );
    g_Render->DrawModel( &tank_turret_model, m_vPos, 0, *m_pColor );
}

/*
===============================================================================

Name    :   cCheckButton

===============================================================================
*/

cCheckButton::cCheckButton (char *szTitle, vec2 vPos, bool *pValue)
{
    strncpy( m_szTitle, szTitle, 64 );
    m_vPos = vPos;
    m_pValue = pValue;

    m_bOver = false;
    m_bClicked = false;
}

bool cCheckButton::Click (vec2 vCursorPos, bool bDown)
{
    if ( over(vCursorPos,m_vPos,vec2(12,12)) )
    {
        if (bDown)
            *m_pValue ^= 1;
    }

    return false;
}

void cCheckButton::Draw (vec2 vCursorPos)
{
    int nBoxIn, nBoxOut;

    if ( over( vCursorPos, m_vPos, vec2(12,12)) )
        nBoxOut = 6;
    else
        nBoxOut = 4;

    if (*m_pValue)
        nBoxIn = 6;
    else
        nBoxIn = 3;

    g_Render->DrawBox( vec2(12,12), m_vPos, 0, menu_colors[nBoxOut] );
    g_Render->DrawBox( vec2(10,10), m_vPos, 0, menu_colors[nBoxIn] );
    g_Render->DrawString( m_szTitle, vec2(m_vPos.x+10,m_vPos.y+4), menu_colors[7] );
}


/*
===============================================================================

Name    :   cServerButton

===============================================================================
*/

cServerButton::cServerButton (vec2 vPos, vec2 vSize, char *szServer, float *flPing, func_t op_click)
{
    m_szServer = szServer;
    m_flPing = flPing;

    m_vPos = vPos;
    m_vSize = vSize;
    m_op_click = op_click;
}

bool cServerButton::Click (vec2 vCursorPos, bool bDown)
{
    m_bOver = over( vCursorPos, vec2(m_vPos+vec2(m_vSize.x/2-18,0)), vec2(32,m_vSize.y-4) );

    if ( !m_szServer || !m_szServer[0] )
    {
        m_bClicked = false;
        m_bOver = false;

        return false;
    }

    if (m_bOver && bDown)
    {
        m_bClicked = true;
    }
    else if (m_bClicked && m_bOver && !bDown)
    {
        m_bClicked = false;
        m_op_click( );
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cServerButton::Draw (vec2 vCursorPos)
{
    int     nButtonIn;
    int     nButtonOut;
    int     nText;

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[4] );
    g_Render->DrawBox( m_vSize-vec2(2,2), m_vPos, 0, menu_colors[3] );
    g_Render->DrawBox( m_vSize-vec2(38,4), m_vPos-vec2(17,0), 0, menu_colors[2] );
    g_Render->DrawBox( m_vSize-vec2(40,6), m_vPos-vec2(17,0), 0, menu_colors[0] );

    // join button

    if ( m_szServer && m_szServer[0] )
    {
        g_Render->DrawString( m_szServer, m_vPos-vec2(m_vSize.x/2-4,-4), menu_colors[7] );
        g_Render->DrawString( va("%i", (int)(*m_flPing)), m_vPos+vec2(m_vSize.x/2-64,4), menu_colors[7] );

        if ( over( vCursorPos, vec2(m_vPos+vec2(m_vSize.x/2-18,0)), vec2(32,m_vSize.y-4) ) )
            nButtonOut = 6;
        else
            nButtonOut = 4;

        if ( m_bClicked )
            nButtonIn = 3;
        else
            nButtonIn = 5;

        nText = 7;
    }
    else
    {
        nButtonIn = 4;
        nButtonOut = 2;
        nText = 2;
    }

    g_Render->DrawBox( vec2(32,m_vSize.y-4), m_vPos+vec2(m_vSize.x/2-18,0), 0, menu_colors[nButtonOut] );
    g_Render->DrawBox( vec2(30,m_vSize.y-6), m_vPos+vec2(m_vSize.x/2-18,0), 0, menu_colors[nButtonIn] );
    g_Render->DrawString( "Join", m_vPos+vec2(m_vSize.x/2-26,4), menu_colors[nText] );

}

/*
===============================================================================

Name    :   cHostButton

===============================================================================
*/

cHostButton::cHostButton (vec2 vPos, vec2 vSize, func_t op_click)
{
    m_vPos = vPos;
    m_vSize = vSize;
    m_op_click = op_click;
}

bool cHostButton::Click (vec2 vCursorPos, bool bDown)
{
    m_bOver = over( vCursorPos, vec2(m_vPos+vec2(m_vSize.x/2-18,0)), vec2(32,m_vSize.y-4) );

    if (m_bOver && bDown)
    {
        m_bClicked = true;
    }
    else if (m_bClicked && m_bOver && !bDown)
    {
        m_bClicked = false;
        m_op_click( );
    }
    else if ( over( vCursorPos, m_vPos, m_vSize ) )
    {
        if ( bDown )
            g_Game->bServerButton ^= true;
    }
    else if (!bDown)
    {
        m_bClicked = false;
    }

    return false;
}

void cHostButton::Draw (vec2 vCursorPos)
{
    int     nButtonIn;
    int     nButtonOut;

    int     nInner;

    if ( g_Game->bServerButton )
        nInner = 5;
    else
        nInner = 2;

    g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[4] );
    g_Render->DrawBox( m_vSize-vec2(2,2), m_vPos, 0, menu_colors[3] );
    g_Render->DrawBox( m_vSize-vec2(38,4), m_vPos-vec2(17,0), 0, menu_colors[nInner] );

    // create button

    g_Render->DrawString( g_Game->svs.name, m_vPos-vec2(m_vSize.x/2-4,-4), menu_colors[7] );

    if ( over( vCursorPos, vec2(m_vPos+vec2(m_vSize.x/2-18,0)), vec2(32,m_vSize.y-4) ) )
        nButtonOut = 6;
    else
        nButtonOut = 4;

    if ( m_bClicked )
        nButtonIn = 3;
    else
        nButtonIn = 5;

    g_Render->DrawBox( vec2(32,m_vSize.y-4), m_vPos+vec2(m_vSize.x/2-18,0), 0, menu_colors[nButtonOut] );
    g_Render->DrawBox( vec2(30,m_vSize.y-6), m_vPos+vec2(m_vSize.x/2-18,0), 0, menu_colors[nButtonIn] );
    g_Render->DrawString( "Create", m_vPos+vec2(m_vSize.x/2-32,4), menu_colors[7] );

}
