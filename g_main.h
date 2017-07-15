// g_main.h
//

#pragma once

namespace render {
class image;
class system;
} // namespace render

#define FRAMETIME   0.05f
#define FRAMEMSEC   50.0f

#define RESTART_TIME    5000.0f

#define SPAWN_BUFFER    32

#define MAX_PLAYERS 16

#define PROTOCOL_VERSION    4

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
typedef struct game_client_s
{
    float   damage_mod;
    float   armor_mod;
    float   refire_mod;
    float   speed_mod;

    int     upgrades;

    usercmdgen input;
} game_client_t;

//------------------------------------------------------------------------------
constexpr color3 player_colors[] = {
    { 1.000f, 0.000f, 0.000f },     // 0: red
    { 0.000f, 0.000f, 1.000f },     // 1: blue
    { 0.000f, 1.000f, 0.000f },     // 2: green
    { 1.000f, 1.000f, 0.000f },     // 3: yellow
    { 1.000f, 0.500f, 0.000f },     // 4: orange
    { 1.000f, 0.500f, 1.000f },     // 5: pink
    { 0.000f, 1.000f, 1.000f },     // 6: cyan
    { 1.000f, 1.000f, 1.000f },     // 7: white
    { 0.500f, 0.000f, 1.000f },     // 8: purple
    { 0.750f, 0.750f, 0.500f },     // 9: tan
    { 0.625f, 0.000f, 0.000f },     // 10: dark red
    { 0.000f, 0.000f, 0.625f },     // 11: dark blue
    { 0.000f, 0.625f, 0.000f },     // 12: dark green
    { 0.250f, 0.375f, 0.000f },     // 13: dark yellow-green
    { 0.750f, 0.125f, 0.012f },     // 14: crimson
    { 0.000f, 0.500f, 1.000f },     // 15: light blue
    { 0.000f, 0.500f, 0.500f },     // 16: teal
    { 0.375f, 0.375f, 0.375f },     // 17: gray
    { 0.200f, 0.000f, 0.500f },     // 18: dark purple
    { 0.375f, 0.250f, 0.125f },     // 19: brown
};
constexpr std::size_t num_player_colors = countof(player_colors);

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
enum class game_mode
{
    singleplayer,
    deathmatch,
    cooperative,
};

//------------------------------------------------------------------------------
typedef struct  message_s
{
    char    string[MAX_STRING];
    float   time;
} message_t;

#define MAX_MESSAGES    32

//
// SERVER SIDE DATA
//

//------------------------------------------------------------------------------
typedef struct client_s
{
    bool    active;
    bool    local;

    network::channel   netchan;

    char name[SHORT_STRING];
    color3 color;
} client_t;

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
typedef struct remote_server_s
{
    char        name[SHORT_STRING];
    network::address    address;

    float       ping;
    bool        active;
} remote_server_t;

#define MAX_SERVERS 8

//------------------------------------------------------------------------------
typedef struct client_state_s
{
    color3  color;
    char    name[SHORT_STRING];
    int     number;

    char    server[SHORT_STRING];
    int     last_frame;

    float           ping_time;
    remote_server_t servers[MAX_SERVERS];
} client_state_t;

//------------------------------------------------------------------------------
class session : public vMain
{
public:
    session();
    ~session() {}

    int init (char const *cmdline);
    int shutdown ();

    void init_client();
    void shutdown_client();

    virtual int message(char const* message, ...);

    int run_frame(float milliseconds);

    int key_event(unsigned char key, bool down);

    void reset();
    void new_game();
    void restart();
    void resume();

    void add_score(int player_index, int score);

    bool _menu_active;
    bool _game_active;

    bool _multiplayer;     // is multiplayer
    bool _multiserver;     // hosting multiplayer
    bool _have_server;      // found a server
    bool _multiplayer_active;     // active and playing
    bool _dedicated;

    float _frametime;
    int _framenum;

    game_mode _mode;

    bool _extended_armor;
    bool _random_spawn;

    float _restart_time;

    static int find_server_by_name(void *lpvoid);

    game::tank* player( int index ) { return _players[ index ]; }

private:
    menu::window _menu;
    game::world _world;

    config::system* _config;

    render::system* _renderer;

    render::image const* _menu_image;

    config::scalar _upgrade_frac;
    config::scalar _upgrade_penalty;
    config::scalar _upgrade_min;
    config::boolean _upgrades;

    config::string _net_master;
    config::string _net_server_name;

    config::string _cl_name;
    config::string _cl_color;

    void get_cursor();
    vec2 _cursor;

    void update_screen();

    int _score[MAX_PLAYERS];
    void draw_score ();

    friend game::tank;

    game::tank* _players[MAX_PLAYERS];
    void spawn_player(int num);
    void respawn_player(int num);

    message_t _messages[MAX_MESSAGES];
    int _num_messages;

    void draw_messages ();

    char _shift_keys[256];

public:
    void write_message (char const* message, bool broadcast=true);
    void write_message_client(char const* message) { write_message(message, false); }

    game_client_t _clients[MAX_PLAYERS];
    // NETWORKING

public:
    void start_server();
    void stop_server();

    void stop_client();

    void connect_to_server(int index);

    void info_ask();

    void write_sound(int sound_index);
    void write_effect(int type, vec2 pos, vec2 vel, float strength);

    client_state_t  cls;
    server_state_t  svs;

    bool _client_button_down;
    bool _server_button_down;
    bool _client_say;

private:
    void get_packets ();
    void get_frame ();
    void write_frame ();
    void send_packets ();

    void broadcast (int len, byte *data);
    void broadcast_print (char const* message);

    void connectionless (network::socket socket);
    void packet (network::socket socket);

    void connect_ack ();

    void client_connect ();
    void client_disconnect (int nClient);
    void client_command ();

    void read_upgrade (int index);
    void write_upgrade (int upgrade);

    void read_sound ();
    void read_effect ();

    void client_send ();

    void info_send ();
    void info_get ();

    void read_info ();
    void write_info (int client, network::message *message);

    void read_fail ();

    network::address _netserver;

    network::channel _netchan;
    network::address _netfrom;

    byte _netmsgbuf[MAX_MSGLEN];
    network::message _netmsg;

    int _netclient;
    char* _netstring;

    char _clientsay[LONG_STRING];
};

} // namespace game

extern game::session* g_Game;
