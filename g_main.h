/*
===============================================================================

Name	:	g_main.h

Purpose	:	Game Object

Date	:	10/20/2004

===============================================================================
*/

class cInputObject
{
public:
	virtual void	Key_Event (int nKey, bool bDown) = 0;
};

#include "scr_main.h"
#include "scr_tank.h"
#include "g_menu.h"
#include "g_world.h"

#define MAX_PLAYERS	2

static vec4 player_colors[] = {
	vec4(1.000,0.000,0.000,1),		// 0: red
	vec4(0.000,0.000,1.000,1),		// 1: blue
	vec4(0.000,1.000,0.000,1),		// 2: green
	vec4(1.000,1.000,0.000,1),		// 3: yellow
	vec4(1.000,0.500,0.000,1),		// 4: orange
	vec4(1.000,0.000,1.000,1),		// 5: pink
	vec4(0.000,1.000,1.000,1),		// 6: cyan
	vec4(1.000,1.000,1.000,1),		// 7: white
	vec4(0.000,0.500,0.500,1),		// 8: teal
	vec4(0.500,0.000,1.000,1),		// 9: purple
	vec4(0.375,0.250,0.125,1),		// 10: brown
	vec4(0.750,0.750,0.500,1)		// 11: tan
};
#define	NUM_PLAYER_COLORS	12

enum eGameType
{
	game_singleplay,
	game_deathmatch,
	game_coop
};

/*
===========================================================

Name	:	cPlayer

Purpose	:	stores information about player[x]

===========================================================
*/
#if 0

class cPlayer
{
public:
	cPlayer () {}
	~cPlayer () {}

	char	m_szName[64];
	vec4	m_vColor;
};
#endif
/*
===========================================================

Name	:	cGame

Purpose	:	Stores higher-level information about the current game

===========================================================
*/

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

	eGameType	m_eGameType;

	void	SetInputObject (cInputObject *pInputObject);

private:
	cMenu	m_Menu;
	cWorld	m_World;

	void	m_getCursorPos ();
	vec2	m_vCursorPos;

	int		m_nScore[MAX_PLAYERS];
	void	m_DrawScore ();

	cTank	m_Players[2];

	void	m_Deathmatch_NewRound ();

	void	m_InitPlayers ();

	cInputObject	*m_pInputObject;
};

extern cGame *g_Game;
extern cRender *g_Render;