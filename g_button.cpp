/*
===============================================================================

Name	:	g_button.cpp

Purpose	:	all the button crap for the menus

Date	:	10/29/2004

===============================================================================
*/

#include "local.h"

#define CHAR_WIDTH	2.5

#define over(a,b,c) ( (clamp(a.x,b.x-c.x/2,b.x+c.x/2) == a.x ) && (clamp(a.y,b.y-c.y/2,b.y+c.y/2) == a.y ) )


/*
===========================================================

Name	:	cBaseButton

Purpose	:	Basic Base Button class, 

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

	vInSize.x = m_vSize.x - 4;
	vInSize.y = m_vSize.y - 4;

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;	// len * char_width / 2
	vTextPos.y = m_vPos.y + 4;								// char_height / 2

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

Name	:	mCondButton

Purpose	:	Derived button class

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

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;	// len * char_width / 2
	vTextPos.y = m_vPos.y + 4;								// char_height / 2

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
	g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
	g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

/*
===========================================================

Name	:	cMenuButton

Purpose	:	Derived button class, activates a given submenu

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
	int		nColorIn, nColorOut, nColorText;
	vec2	vInSize, vTextPos;

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

	vInSize.x = m_vSize.x - 4;
	vInSize.y = m_vSize.y - 4;

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;	// len * char_width / 2
	vTextPos.y = m_vPos.y + 4;								// char_height / 2

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
	g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
	g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
}

/*
===========================================================

Name	:	cTankButton

Purpose	:	various modifications on a tank

===========================================================
*/

cTankButton::cTankButton (char *szTitle, vec2 vPos, vec2 vSize, cTank *pTank)
{
	strncpy( m_szTitle, szTitle, 64 );
	m_vPos = vPos;
	m_vSize = vSize;

	m_pTank = pTank;
	m_nTankColor = pTank->nPlayerNum;

	m_vBoxPos.x = vPos.x - vSize.x / 2 + 10;
	m_vBoxPos.y = vPos.y + vSize.y / 2 - 10;
	m_vBoxSize.x = 12;
	m_vBoxSize.y = 12;

	m_vTextPos.x = vPos.x + m_vBoxSize.x / 2;
	m_vTextPos.y = vPos.y + vSize.y / 2 - 10;

	m_vTextSize.x = vSize.x - m_vBoxSize.x - 10;
	m_vTextSize.y = 12;

	m_bOver = false;
	m_bClicked = false;
	m_bWriting = false;
	m_scrCursor = 0;

	m_pTank->m_bComputer = false;
	memset( m_pTank->m_szScript, 0, 64 );
}

void cTankButton::Key_Event (int nKey, bool bDown)
{
	if (!bDown)
		return;

	if (nKey <= 32)
		return;

	if ((nKey == 127) && (m_scrCursor > 0))
	{
		m_pTank->m_szScript[--m_scrCursor] = 0;
		return;
	}
	else if (m_scrCursor < 63)
	{
		m_pTank->m_szScript[m_scrCursor++] = nKey;
		return;
	}
}

bool cTankButton::Click (vec2 vCursorPos, bool bDown)
{
	if ( m_bOver = over( vCursorPos, m_vBoxPos, m_vBoxSize ) )
	{
		if (bDown)
			m_pTank->m_bComputer ^= 1;
		return false;
	}

	m_bOver = m_Over( vCursorPos );

	if ( over( vCursorPos, m_vTextPos, m_vTextSize ) )
	{
		g_Game->SetInputObject( this );
		m_bWriting = true;
		return false;
	}
	else if ( m_bOver )
	{
		g_Game->SetInputObject( NULL );
		m_bWriting = false;
		return false;
	}


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
	int		nColorIn, nColorOut, nColorText;
	vec2	vInSize, vTextPos;

	int		nBoxIn, nBoxOut;
	int		nTextOut;
	int		nTextIn;
	vec2	vBoxSizeIn;
	vec2	vTextSizeIn;
	vec2	vScriptPos;

	m_bOver = m_Over( vCursorPos );

	if (m_bOver)
		nColorOut = 6;
	else
		nColorOut = 4;

	if (m_bClicked)
		nColorIn = 3;
	else
		nColorIn = 5;

	if (m_pTank->m_bComputer)
		nBoxIn = 6;
	else
		nBoxIn = 3;

	if ( over( vCursorPos, m_vBoxPos, m_vBoxSize ) )
		nBoxOut = 6;
	else
		nBoxOut = 4;

	if ( over( vCursorPos, m_vTextPos, m_vTextSize ) )
		nTextOut = 6;
	else
		nTextOut = 4;

	if (m_bWriting)
		nTextIn = 4;
	else
		nTextIn = 3;

	nColorText = nColorIn + 2;

	vInSize.x = m_vSize.x - 4;
	vInSize.y = m_vSize.y - 4;

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;	// len * char_width / 2
	vTextPos.y = m_vPos.y - m_vSize.y / 2 + 16;

	vBoxSizeIn.x = m_vBoxSize.x - 2;
	vBoxSizeIn.y = m_vBoxSize.y - 2;

	vTextSizeIn.x = m_vTextSize.x - 2;
	vTextSizeIn.y = m_vTextSize.y - 2;

	vScriptPos.x = m_vTextPos.x - m_vTextSize.x / 2 + 2;
	vScriptPos.y = m_vTextPos.y + m_vTextSize.y / 2 - 2;

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
	g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
	g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );

	g_Render->DrawBox( m_vBoxSize, m_vBoxPos, 0, menu_colors[nBoxOut] );
	g_Render->DrawBox( vBoxSizeIn, m_vBoxPos, 0, menu_colors[nBoxIn] );

	g_Render->DrawBox( m_vTextSize, m_vTextPos, 0, menu_colors[nTextOut] );
	g_Render->DrawBox( vTextSizeIn, m_vTextPos, 0, menu_colors[nTextIn] );
	g_Render->DrawString( m_pTank->m_szScript, vScriptPos, menu_colors[nColorText] );

	g_Render->DrawModel( m_pTank->pModel, m_vPos, 0, m_pTank->vColor );
	g_Render->DrawModel( m_pTank->pTurret, m_vPos, 0, m_pTank->vColor );
}

/*
===========================================================

Name	:	cRangeButton

Purpose	:	range

===========================================================
*/

cRangeButton::cRangeButton (char *szTitle, vec2 vPos, vec2 vSize, int nMax, int *nVal)
{
	strncpy( m_szTitle, szTitle, 64 );
	m_vPos = vPos;
	m_vSize = vSize;

	m_nMax = nMax;
	m_nVal = nVal;
	m_bOver = false;
	m_bClicked = false;
}

bool cRangeButton::Click (vec2 vCursorPos, bool bDown)
{
	m_bOver = m_Over( vCursorPos );

	if (m_bOver && bDown)
	{
		m_bClicked = true;
	}
	else if (m_bClicked && m_bOver && !bDown)
	{
		m_bClicked = false;
		*m_nVal = (*m_nVal+1)%m_nMax;
	}
	else if (!bDown)
	{
		m_bClicked = false;
	}

	return false;
}

void cRangeButton::Draw (vec2 vCursorPos)
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

	vInSize.x = m_vSize.x - 4;
	vInSize.y = m_vSize.y - 4;

	vTextPos.x = m_vPos.x - strlen(m_szTitle)*CHAR_WIDTH;	// len * char_width / 2
	vTextPos.y = m_vPos.y + 4;								// char_height / 2

	g_Render->DrawBox( m_vSize, m_vPos, 0, menu_colors[nColorOut] );
	g_Render->DrawBox( vInSize, m_vPos, 0, menu_colors[nColorIn] );
	g_Render->DrawString( m_szTitle, vTextPos, menu_colors[nColorText] );
	vTextPos.y += 12;
	g_Render->DrawString( va("%i",*m_nVal), vTextPos, menu_colors[nColorText] );
}
