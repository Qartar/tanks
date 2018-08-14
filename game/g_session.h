// g_session.h
//

#pragma once

#include "cm_time.h"
#include "net_channel.h"
#include "net_socket.h"

namespace render {
class image;
class system;
} // namespace render

constexpr const time_delta RESTART_TIME = time_delta::from_seconds(5.0f);

#define SPAWN_BUFFER    32

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

    time_value usercmd_time;

    static constexpr time_delta usercmd_rate = time_delta::from_hertz(60.0f);
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
    svc_snapshot,   //  game snapshot
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
    char string[MAX_STRING];
    time_value time;
} message_t;

#define MAX_MESSAGES    32

//------------------------------------------------------------------------------
struct userinfo
{
    std::array<char, 64> name;
    color3 color;
    weapon_type weapon;
};

//
// SERVER SIDE DATA
//

//------------------------------------------------------------------------------
typedef struct client_s
{
    bool active; //!< client is connected and active
    bool local; //!< client is local (host or hotseat)

    network::channel netchan;
    game::userinfo info;
} client_t;

//------------------------------------------------------------------------------
typedef struct server_state_s
{
    bool active; //!< server is connected and active
    bool local; //!< server is local only (hotseat)

    char        name[SHORT_STRING];

    std::array<client_t, MAX_PLAYERS> clients;

    network::socket socket;
} server_state_t;

//
// CLIENT SIDE DATA
//

//------------------------------------------------------------------------------
typedef struct remote_server_s
{
    char        name[SHORT_STRING];
    network::address    address;

    time_delta  ping;
    bool        active;
} remote_server_t;

#define MAX_SERVERS 8

//------------------------------------------------------------------------------
typedef struct client_state_s
{
    bool active; //!< client is connected and active
    bool local; //!< client is local (host or hotseat)

    game::userinfo info;

    int     number;

    char    server[SHORT_STRING];

    time_value      ping_time;
    remote_server_t servers[MAX_SERVERS];

    network::socket socket;
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

    int run_frame(time_delta time);

    void key_event(int key, bool down);
    void cursor_event(vec2 position);
    void gamepad_event(int index, gamepad const& pad);

    void reset();
    void new_game();
    void restart();
    void resume();

    void add_score(int player_index, int score);

    bool _menu_active;
    bool _dedicated;

    static int find_server_by_name(void *lpvoid);

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
    config::integer _cl_weapon;

    game_mode _mode;

    time_value _restart_time;

    time_value _worldtime;
    time_value _frametime;
    int _framenum;

    vec2i _cursor;
    bool _show_cursor;

    void update_screen();

    void draw_world();

    void draw_menu();

    int _score[MAX_PLAYERS];
    void draw_score ();

    void draw_netgraph();

    void spawn_player(int num);

    message_t _messages[MAX_MESSAGES];
    int _num_messages;

    void draw_messages ();

    char _shift_keys[256];

    config::boolean _net_graph;
    std::array<int, 256> _net_bytes;

public:
    void write_message (char const* message, bool broadcast=true);
    void write_message_client(char const* message) { write_message(message, false); }

    game_client_t _clients[MAX_PLAYERS];
    // NETWORKING

public:
    void start_server();
    void start_server_local();
    void stop_server();

    void start_client_local();
    void stop_client();

    void connect_to_server(int index);
    void connect_to_server(char const* address);

    void info_ask();

    client_state_t  cls;
    server_state_t  svs;

    bool _client_button_down;
    bool _server_button_down;
    bool _client_say;

private:
    void get_packets ();
    void read_snapshot(network::message& message);
    void write_frame ();
    void send_packets ();

    void broadcast(int len, byte const* data);
    void broadcast(network::message& message);
    void broadcast_print (char const* message);

    void server_connectionless(network::address const& remote, network::message& message);
    void client_connectionless(network::address const& remote, network::message& message);

    void server_packet(network::message& message, int client);
    void client_packet(network::message& message);

    void connect_ack(char const* message_string);

    void client_connect(network::address const& remote, char const* message_string);
    void client_connect(network::address const& remote, char const* message_string, int client);
    void client_disconnect(int client);
    void client_command(network::message& message, int client);

    void read_upgrade(int client, int upgrade);
    void write_upgrade(int upgrade);

    void client_send ();

    void info_send(network::address const& remote);
    void info_get(network::address const& remote, char const* message_string);

    void read_info(network::message& message);
    void write_info(network::message& message, int client);

    void read_fail(char const* message_string);

    network::address _netserver;

    network::channel _netchan;

    char _clientsay[LONG_STRING];
};

} // namespace game

extern game::session* g_Game;
