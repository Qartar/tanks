/*
===============================================================================

Name    :   g_main.h

Purpose :   Game Object

Date    :   10/20/2004

===============================================================================
*/

#pragma once

#define FRAMETIME   0.05f
#define FRAMEMSEC   50.0f

#define RESTART_TIME    5000.0f

#define SPAWN_BUFFER    32

#define MAX_PLAYERS 16

#define KEY_FORWARD 0
#define KEY_BACK    1
#define KEY_LEFT    2
#define KEY_RIGHT   3
#define KEY_TLEFT   4
#define KEY_TRIGHT  5
#define KEY_FIRE    6

#define BIT(a)  (1<<a)

static vec4 player_colors[] = {
    vec4(   1.000f, 0.000f, 0.000f, 1),     // 0: red
    vec4(   0.000f, 0.000f, 1.000f, 1),     // 1: blue
    vec4(   0.000f, 1.000f, 0.000f, 1),     // 2: green
    vec4(   1.000f, 1.000f, 0.000f, 1),     // 3: yellow
    vec4(   1.000f, 0.500f, 0.000f, 1),     // 4: orange
    vec4(   1.000f, 0.500f, 1.000f, 1),     // 5: pink
    vec4(   0.000f, 1.000f, 1.000f, 1),     // 6: cyan
    vec4(   1.000f, 1.000f, 1.000f, 1),     // 7: white
    vec4(   0.500f, 0.000f, 1.000f, 1),     // 8: purple
    vec4(   0.750f, 0.750f, 0.500f, 1),     // 9: tan
    vec4(   0.625f, 0.000f, 0.000f, 1),     // 10: dark red
    vec4(   0.000f, 0.000f, 0.625f, 1),     // 11: dark blue
    vec4(   0.000f, 0.625f, 0.000f, 1),     // 12: dark green
    vec4(   0.250f, 0.375f, 0.000f, 1),     // 13: dark yellow-green
    vec4(   0.750f, 0.125f, 0.012f, 1),     // 14: crimson
    vec4(   0.000f, 0.500f, 1.000f, 1),     // 15: light blue
    vec4(   0.000f, 0.500f, 0.500f, 1),     // 16: teal
    vec4(   0.375f, 0.375f, 0.375f, 1),     // 17: gray
    vec4(   0.200f, 0.000f, 0.500f, 1),     // 18: dark purple
    vec4(   0.375f, 0.250f, 0.125f, 1),     // 19: brown
};
#define NUM_PLAYER_COLORS   20

typedef enum netops_e
{
    net_bad,
    net_nop,

    clc_command,    //  player commands
    clc_disconnect, //  disconnected
    clc_say,        //  message text
    clc_upgrade,    //  upgrade command

    svc_disconnect, //  force disconnect
    svc_message,    //  message from server
    svc_score,      //  score update
    svc_info,       //  client info
    svc_frame,      //  frame update
    svc_effect,     //  particle effect
    svc_sound,      //  sound effect
    svc_restart     //  game restart
} netops_t;

enum eGameType
{
    game_singleplay,
    game_deathmatch,
    game_coop
};

typedef struct  message_s
{
    char    string[MAX_STRING];
    float   time;
} message_t;

#define MAX_MESSAGES    32

/*
===========================================================

Name    :   cGame

Purpose :   Stores higher-level information about the current game

===========================================================
*/

class cMenu;
class cWorld;
class cRender;

//
// SERVER SIDE DATA
//

typedef struct client_s
{
    bool    active;
    bool    local;

    netchan_t   netchan;

    char        name[SHORT_STRING];
} client_t;

typedef struct server_state_s
{
    bool    active;

    char        name[SHORT_STRING];

    client_t    clients[MAX_PLAYERS];
    int         max_clients;
} server_state_t;

//
// CLIENT SIDE DATA
//

typedef struct remote_server_s
{
    char        name[SHORT_STRING];
    netadr_t    address;

    float       ping;
    bool        active;
} remote_server_t;

#define MAX_SERVERS 8

typedef struct client_state_s
{
    vec4    color;
    char    name[SHORT_STRING];
    int     number;

    char    server[SHORT_STRING];
    int     last_frame;

    float           ping_time;
    remote_server_t servers[MAX_SERVERS];
} client_state_t;

// SHARED DATA

// if your brain hasn't exploded yet it will now

class cGame : public vMain
{
    friend  cMenu;

public:
    cGame ();
    ~cGame () {}

    int     Init (char *cmdline);
    int     Shutdown ();

    void    m_InitClient ();
    void    m_EndClient ();

    virtual int Message (char *szMessage, ...);

    int     RunFrame (float flMSec);

    int     Key_Event (unsigned char Key, bool Down);

    void    Reset ();
    void    NewGame ();
    void    Restart ();
    void    Resume ();

    void    AddScore (int nPlayer, int nScore);

    bool    m_bMenuActive;
    bool    m_bGameActive;

    bool    m_bMultiplayer;     // is multiplayer
    bool    m_bMultiserver;     // hosting multiplayer
    bool    m_bHaveServer;      // found a server
    bool    m_bMultiactive;     // active and playing
    bool    m_bDedicated;

    float   m_flTime;
    int     m_nFramenum;

    eGameType   m_eGameType;

    bool    bExtendedArmor;
    bool    bRandomSpawn;
    bool    bAutoRestart;
    bool    bManualRestart;

    float   flRestartTime;

    static int  FindServerByName (void *lpvoid);

    game::tank *     Player( int index ) { return m_Players[ index ]; }

private:
    cMenu   m_Menu;
    game::world m_World;

    int     menuImage;

    void    m_getCursorPos ();
    vec2    m_vCursorPos;

    int     m_nScore[MAX_PLAYERS];
    void    m_DrawScore ();

    game::tank* m_Players[MAX_PLAYERS];
    void spawn_player(int num);
    void respawn_player(int num);

    message_t   m_Messages[MAX_MESSAGES];
    int         m_nMessage;

    void    m_DrawMessages ();

    char    m_ShiftKeys[256];

public:
    void    m_WriteMessage (char *szMessage, bool broadcast=true);
    void    m_WriteMessageClient( char *szMessage ) { m_WriteMessage( szMessage, false ); }

    game_client_t   gameClients[MAX_PLAYERS];
    // NETWORKING

public:
    void    m_StartServer ();
    void    m_StopServer ();

    void    m_StopClient ();

    void    m_ConnectToServer (int index);

    void    m_InfoAsk ();

    void    m_WriteSound (int nSound);
    void    m_WriteEffect (int type, vec2 pos, vec2 vel, int count);

    client_state_t  cls;
    server_state_t  svs;

    bool    bClientButton;
    bool    bServerButton;
    bool    bClientSay;

private:
    void    m_GetPackets ();
    void    m_GetFrame ();
    void    m_WriteFrame ();
    void    m_SendPackets ();

    void    m_Broadcast (int len, byte *data);
    void    m_Broadcast_Print (char *message);

    void    m_Connectionless (netsock_t socket);
    void    m_Packet (netsock_t socket);

    void    m_ConnectAck ();

    void    m_ClientConnect ();
    void    m_ClientDisconnect (int nClient);
    void    m_ClientCommand ();

    void    m_ReadUpgrade (int index);
    void    m_WriteUpgrade (int upgrade);

    void    m_ReadSound ();
    void    m_ReadEffect ();

    void    m_ClientKeys (int key, bool down);
    void    m_clientsend ();

    void    m_InfoSend ();
    void    m_InfoGet ();

    void    m_ReadInfo ();
    void    m_WriteInfo (int client, netmsg_t *message);

    void    m_ReadFail ();

    bool    m_clientkeys[8];

    netadr_t    m_netserver;

    netchan_t   m_netchan;
    netadr_t    m_netfrom;

    byte        m_netmsgbuf[MAX_MSGLEN];
    netmsg_t    m_netmsg;

    int         m_netclient;
    char        *m_netstring;

    char        m_clientsay[LONG_STRING];
};

struct index_s
{
    char    *name;
    int     index;
};
extern index_s  sound_index[256];

extern cGame *g_Game;
extern cRender *g_Render;
