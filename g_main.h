/*
===============================================================================

Name	:	g_main.h

Purpose	:	Game Object

Date	:	10/20/2004

===============================================================================
*/

#include "g_menu.h"
#include "g_world.h"

#define MAX_PLAYERS	2

class cGame
{
	friend	cMenu;

public:
	cGame () {}
	~cGame () {}

	int		Init ();
	int		Shutdown ();

	int		RunFrame (float flMSec);

	int		Key_Event (unsigned char Key, bool Down);

	void	Reset ();
	void	NewGame ();
	void	Resume ();

	void	AddScore (int nPlayer, int nScore);

	bool	m_bMenuActive;
	bool	m_bGameActive;

	float	m_flTime;
	int		m_nFramenum;

private:
	cMenu	m_Menu;
	cWorld	m_World;
	cRender	*m_pRender;

	void	m_getCursorPos ();
	vec2	m_vCursorPos;

	int		m_nScore[MAX_PLAYERS];
	void	m_DrawScore ();

	cTank	m_Players[2];
};

extern cGame *g_Game;