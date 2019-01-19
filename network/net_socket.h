// net_socket.h
//

#pragma once

#include "cm_string.h"

struct sockaddr_storage;

////////////////////////////////////////////////////////////////////////////////
namespace network {

class address;
class message;

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
    bool printf(network::address const& remote, string::literal fmt, ...);

    //! resolve the string into an address
    bool resolve(string::view address_string, network::address& address) const;

protected:
    socket_type _type;
    socket_port _port;
    std::uintptr_t _socket;

protected:
    socket(socket const&) = delete;
    socket& operator=(socket const&) = delete;

    bool sockaddr_to_address(sockaddr_storage const& sockaddr, network::address& address) const;
    bool address_to_sockaddr(network::address const& address, sockaddr_storage& sockaddr) const;
    bool resolve_sockaddr(string::view address_string, sockaddr_storage& sockaddr) const;

    std::uintptr_t open_socket(socket_type type, word port = socket_port::any) const;
};

} // namespace network
