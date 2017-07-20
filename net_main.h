// net_main.h
//

#pragma once

#include "shared.h"

#include <array>

struct sockaddr_storage;

#define MAX_MSGLEN  1400

////////////////////////////////////////////////////////////////////////////////
namespace network {

enum address_type { loopback, broadcast, ip4, ip6 };
enum socket { client, server, NUM_SOCKETS };

//------------------------------------------------------------------------------
class address
{
public:
    address_type type;

    union {
        std::array<byte, 4> ip4;
        std::array<word, 8> ip6;
    };

    unsigned short port;

public:
    address() = default;
    constexpr address(byte const (&ip)[4], word port = 0)
        : type(address_type::ip4)
        , ip4({ip[0], ip[1], ip[2], ip[3]})
        , port(port)
    {}
    constexpr address(word const (&ip)[8], word port = 0)
        : type(address_type::ip6)
        , ip6({ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]})
        , port(port)
    {}

    bool operator==(network::address const& other) const;
    bool operator!=(network::address const& other) const { return !(*this == other); }
    bool is_local() const { return (type == network::address_type::loopback); }
};

//------------------------------------------------------------------------------
class message
{
public:
    byte* data;
    int size;
    int bytes_written;
    int bytes_remaining;
    int bytes_read;
    bool allow_overflow;
    bool overflowed;

    void* alloc(int nSize);

    void init(byte *pData, int nMaxSize);
    void clear();
    void write(void const* pData, int nSize);
    int read(void *pOut, int nSize);

    void write_byte(int b);
    void write_short(int s);
    void write_long(int l);
    void write_float(float f);
    void write_char(int c);
    void write_string(char const* sz);
    void write_angle(float f);
    void write_vector(vec2 v);

    void begin ();

    int read_byte();
    int read_short();
    int read_long();
    float read_float();
    int read_char();
    char* read_string();
    char* read_line();
    float read_angle();
    vec2 read_vector();
};

//------------------------------------------------------------------------------
class channel
{
public:
    int init(int netport = 0);
    int setup(network::socket socket, network::address remote, int netport = 0);

    int transmit(int nLength, byte *pData);
    int process(network::message *pMessage);

    network::socket socket;

    network::address address;    //  remote address
    int netport;    //  port translation

    network::message message;    // outgoing
    byte messagebuf[MAX_MSGLEN];

    float last_sent;
    float last_received;

    float connect_time;
    int connect_tries;
};

//------------------------------------------------------------------------------
typedef struct loopmsg_s
{
    byte    data[MAX_MSGLEN];
    int size;
} loopmsg_t;

#define MAX_LOOPBACK    4

//------------------------------------------------------------------------------
typedef struct loopback_s
{
    loopmsg_t   msgs[MAX_LOOPBACK];
    int     get, send;
} loopback_t;

//------------------------------------------------------------------------------
class manager
{
public:
    friend channel;

    int init();
    int shutdown();

    int config(bool multiplayer);

    int print(network::socket socket, network::address to, char const *szMessage);

    int get(network::socket socket, network::address *pFrom, network::message *pMessage);

    bool string_to_address(char const *addr, network::address *net);
    char const* address_to_string(network::address a);

private:
    int send(network::socket socket, int nLength, void const *pData, network::address to);

    int get_loopback(network::socket socket, network::address *pFrom, network::message *pMessage);
    int send_loopback(network::socket socket, int nLength, void const *pData, network::address to);

    loopback_t  _loopbacks[NUM_SOCKETS];

    bool _multiplayer;

    // winsock

    char const* WSAErrorString (int code = 0);

    int _ip4_sockets[NUM_SOCKETS];
    int _ip6_sockets[NUM_SOCKETS];

    constexpr static network::address _ip6_broadcast = {{0xff02, 0, 0, 0, 0, 0, 0, 1}, 0};
    constexpr static network::address _ip6_loopback = {{0, 0, 0, 0, 0, 0, 0, 1}, 0};

    int open_ip4();
    int open_ip6();

    int ip4_socket(int port);
    int ip6_socket(int port);

    void address_to_sockaddr(network::address *net, sockaddr_storage *sock);
    void sockaddr_to_address(sockaddr_storage *sock, network::address *net);
    bool string_to_sockaddr(char const *addr, sockaddr_storage *sock);
};

} // namespace network

extern network::manager *pNet;
