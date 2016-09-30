/*
===============================================================================

Name	:	g_main.h

Purpose	:	Game Object

Date	:	10/20/2004

===============================================================================
*/

#include "g_menu.h"
#include "g_world.h"

#define FRAMETIME	0.05f
#define FRAMEMSEC	50.0f

#define RESTART_TIME	3000.0f

#define SPAWN_BUFFER	32

#define MAX_PLAYERS	8

#define KEY_FORWARD	0
#define KEY_BACK	1
#define KEY_LEFT	2
#define KEY_RIGHT	3
#define KEY_TLEFT	4
#define KEY_TRIGHT	5
#define KEY_FIRE	6

#define BIT(a)	(1<<a)

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

typedef enum netops_e
{
	net_bad,
	net_nop,

	clc_command,
	clc_disconnect,
	clc_say,

	svc_disconnect,
	svc_message,
	svc_score,
	svc_info,		// client info
	svc_frame,
	svc_effect
} netops_t;

typedef enum eEffects
{
	effect_smoke,
	effect_sparks,
	effect_explosion
} effects_t;


enum eGameType
{
	game_singleplay,
	game_deathmatch,
	game_coop
};

typedef struct	message_s
{
	char	string[MAX_STRING];
	float	time;
} message_t;

#define MAX_MESSAGES	8

/*
===========================================================

Name	:	cGame

Purpose	:	Stores higher-level information about the current game

===========================================================
*/

typedef struct client_s
{
	bool	active;
	bool	local;

	netchan_t	netchan;

	char		name[SHORT_STRING];
} client_t;

typedef struct client_state_s
{
	vec4	color;
	char	name[SHORT_STRING];

	int		last_frame;
} client_state_t;

class cGame
{
	friend	cMenu;

public:
	cGame () {}
	~cGame () {}

	int		Init (char *cmdline);
	int		Shutdown ();

	int		RunFrame (float flMSec);

	int		Key_Event (unsigned char Key, bool Down);

	void	Reset ();
	void	NewGame ();
	void	Resume ();

	void	AddScore (int nPlayer, int nScore);

	bool	m_bMenuActive;
	bool	m_bGameActive;

	bool	m_bMultiplayer;		// is multiplayer
	bool	m_bMultiserver;		// hosting multiplayer
	bool	m_bHaveServer;		// found a server
	bool	m_bMultiactive;		// active and playing
	bool	m_bDedicated;

	float	m_flTime;
	int		m_nFramenum;

	eGameType	m_eGameType;

	bool	bExtendedArmor;
	bool	bRandomSpawn;
	bool	bAutoRestart;

	float	flRestartTime;

private:
	cMenu	m_Menu;
	cWorld	m_World;

	void	m_getCursorPos ();
	vec2	m_vCursorPos;

	int		m_nScore[MAX_PLAYERS];
	void	m_DrawScore ();

	cTank	m_Players[MAX_PLAYERS];

	void	m_Deathmatch_NewRound ();

	void	m_InitPlayers ();

	message_t	m_Messages[MAX_MESSAGES];
	int			m_nMessage;

	void	m_DrawMessages ();

	char	m_ShiftKeys[256];

public:
	void	m_WriteMessage (char *szMessage);

	// NETWORKING

public:
	void	m_StartServer ();
	void	m_StopServer ();

	void	m_StopClient ();

	void	m_ConnectToServer ();

	void	m_InfoAsk ();

	void	m_WriteEffect (int type, vec2 pos, vec2 vel, int count);

	client_state_t	cls;

	bool	bClientButton;
	bool	bClientSay;

	client_t	m_clients[MAX_PLAYERS];
private:
	void	m_GetPackets ();
	void	m_GetFrame ();
	void	m_WriteFrame ();
	void	m_SendPackets ();

	void	m_Broadcast (int len, byte *data);
	void	m_Broadcast_Print (char *message);

	void	m_Connectionless (netsock_t socket);
	void	m_Packet (netsock_t socket);

	void	m_ConnectAck ();

	void	m_ClientConnect ();
	void	m_ClientDisconnect (int nClient);
	void	m_ClientCommand ();

	void	m_ReadEffect ();

	void	m_ClientKeys (int key, bool down);
	void	m_ClientSend ();

	void	m_InfoSend ();
	void	m_InfoGet ();

	void	m_ReadInfo ();
	void	m_WriteInfo (int client, netmsg_t *message);

	bool	m_clientkeys[8];

	netadr_t	m_netserver;

	netchan_t	m_netchan;
	netadr_t	m_netfrom;

	byte		m_netmsgbuf[MAX_MSGLEN];
	netmsg_t	m_netmsg;

	int			m_netclient;
	char		*m_netstring;

	char		m_clientsay[LONG_STRING];
};

extern cGame *g_Game;
extern cRender *g_Render;