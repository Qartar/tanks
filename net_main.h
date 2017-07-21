// net_main.h
//

#pragma once

#include "shared.h"

#include <array>

struct sockaddr_storage;

#define MAX_MSGLEN  1400

////////////////////////////////////////////////////////////////////////////////
namespace network {

enum address_type { loopback, broadcast, ipv4, ipv6 };

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
        : type(address_type::ipv4)
        , ip4({ip[0], ip[1], ip[2], ip[3]})
        , port(port)
    {}
    constexpr address(word const (&ip)[8], word port = 0)
        : type(address_type::ipv6)
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
enum class socket_type { unspecified, ipv4, ipv6 };
enum socket_port { any = 0 };

//------------------------------------------------------------------------------
class socket
{
public:
    socket();
    socket(socket_type type, word port = socket_port::any);
    ~socket();

    socket(socket&&);
    socket& operator=(socket&&);

    socket_type type() const { return _type; }
    socket_port port() const { return _port; }

    bool valid() const { return _socket != 0; }
    bool open(socket_type type, word port = socket_port::any);
    void close();

    //! read data info message and set remote address
    bool read(network::address& remote, network::message& message);
    //! write data to the remote address
    bool write(network::address const& remote, network::message const& message);
    //! write a formatted string to the remote address
    bool printf(network::address const& remote, char const* fmt, ...);

    //! resolve the string into an address
    bool resolve(std::string const& address_string, network::address& address) const;

protected:
    socket_type _type;
    socket_port _port;
    std::uintptr_t _socket;

protected:
    socket(socket const&) = delete;
    socket& operator=(socket const&) = delete;

    bool sockaddr_to_address(sockaddr_storage const& sockaddr, network::address& address) const;
    bool address_to_sockaddr(network::address const& address, sockaddr_storage& sockaddr) const;
    bool resolve_sockaddr(std::string const& address_string, sockaddr_storage& sockaddr) const;

    std::uintptr_t open_socket(socket_type type, word port = socket_port::any) const;
};

//------------------------------------------------------------------------------
class channel
{
public:
    int init(int netport = 0);
    int setup(network::socket* socket, network::address remote, int netport = 0);

    int transmit(int nLength, byte *pData);
    int process(network::message *pMessage);

    constexpr static int prefix = -1;

    network::address address;    //  remote address
    int netport;    //  port translation

    network::message message;    // outgoing
    byte messagebuf[MAX_MSGLEN];

    float last_sent;
    float last_received;

    float connect_time;
    int connect_tries;

    network::socket* socket;
};

} // namespace network
