// net_main.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "net_main.h"

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <ws2ipdef.h>

network::manager    *pNet;  // extern

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
bool address::operator==(network::address const& other) const
{
    if (type != other.type) {
        return false;
    }

    switch (type) {
        case network::address_type::loopback:
            return true;

        case network::address_type::ip4:
            return ip4 == other.ip4 && port == other.port;

        case network::address_type::ip6:
            return ip6 == other.ip6 && port == other.port;

        default:
            return false;
    }
}

//------------------------------------------------------------------------------
int manager::init ()
{
    WSADATA wsadata = {};

    _multiplayer = false;

    for ( int i=0 ; i<NUM_SOCKETS ; i++ ) {
        memset( &_loopbacks[i], 0, sizeof(_loopbacks[i]) );

        _ip4_sockets[ i ] = 0;
        _ip6_sockets[ i ] = 0;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
        return ERROR_FAIL;
    }

    config( false );

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::shutdown ()
{
    config( false );

    WSACleanup( );

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::config (bool multiplayer)
{
    if ( multiplayer == _multiplayer )
        return ERROR_NONE;

    if ( _multiplayer = multiplayer )
    {
        open_ip4( );
        open_ip6( );
    }
    else
    {
        int     i;

        // shut it down
        for (i=0 ; i<NUM_SOCKETS ; i++)
        {
            if ( _ip4_sockets[i] )
            {
                closesocket( _ip4_sockets[i] );
                _ip4_sockets[i] = 0;
            }

            if ( _ip6_sockets[i] )
            {
                closesocket( _ip6_sockets[i] );
                _ip6_sockets[i] = 0;
            }
        }
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
char const* manager::WSAErrorString (int code)
{
    if ( !code )
        code = WSAGetLastError ();

    switch (code)
    {
    case WSAEINTR: return "WSAEINTR";
    case WSAEBADF: return "WSAEBADF";
    case WSAEACCES: return "WSAEACCES";
    case WSAEDISCON: return "WSAEDISCON";
    case WSAEFAULT: return "WSAEFAULT";
    case WSAEINVAL: return "WSAEINVAL";
    case WSAEMFILE: return "WSAEMFILE";
    case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
    case WSAEINPROGRESS: return "WSAEINPROGRESS";
    case WSAEALREADY: return "WSAEALREADY";
    case WSAENOTSOCK: return "WSAENOTSOCK";
    case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
    case WSAEMSGSIZE: return "WSAEMSGSIZE";
    case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
    case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
    case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
    case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
    case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
    case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
    case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
    case WSAEADDRINUSE: return "WSAEADDRINUSE";
    case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
    case WSAENETDOWN: return "WSAENETDOWN";
    case WSAENETUNREACH: return "WSAENETUNREACH";
    case WSAENETRESET: return "WSAENETRESET";
    case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
    case WSAECONNRESET: return "WSAECONNRESET";
    case WSAENOBUFS: return "WSAENOBUFS";
    case WSAEISCONN: return "WSAEISCONN";
    case WSAENOTCONN: return "WSAENOTCONN";
    case WSAESHUTDOWN: return "WSAESHUTDOWN";
    case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
    case WSAETIMEDOUT: return "WSAETIMEDOUT";
    case WSAECONNREFUSED: return "WSAECONNREFUSED";
    case WSAELOOP: return "WSAELOOP";
    case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
    case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
    case WSASYSNOTREADY: return "WSASYSNOTREADY";
    case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
    case WSANOTINITIALISED: return "WSANOTINITIALISED";
    case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
    case WSATRY_AGAIN: return "WSATRY_AGAIN";
    case WSANO_RECOVERY: return "WSANO_RECOVERY";
    case WSANO_DATA: return "WSANO_DATA";
    case WSAEHOSTUNREACH: return "WSAEHOSTUNREACH";
    default: return "NO ERROR";
    }
}

//------------------------------------------------------------------------------
int manager::open_ip4 ()
{
    if (!_ip4_sockets[network::socket::server]) {
        _ip4_sockets[network::socket::server] = ip4_socket(PORT_SERVER);
    }

    if (!_ip4_sockets[network::socket::client]) {
        if (!(_ip4_sockets[network::socket::client] = ip4_socket(PORT_CLIENT))) {
            _ip4_sockets[network::socket::client] = ip4_socket(0);
        }
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::ip4_socket (int port)
{
    sockaddr_in     address;
    int     newsocket;
    unsigned long       args;

    // get socket
    if ( (newsocket = ::socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        return 0;
    }

    // disable blocking
    if (ioctlsocket (newsocket, FIONBIO, &args) == SOCKET_ERROR) {
        return 0;
    }

    // enable broadcasting
    if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&args, sizeof(args)) == SOCKET_ERROR) {
        return 0;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( (unsigned short )port );

    if (bind (newsocket, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        closesocket( newsocket );
        return 0;
    }

    return newsocket;
}

//------------------------------------------------------------------------------
int manager::open_ip6 ()
{
    if (!_ip6_sockets[network::socket::server]) {
        _ip6_sockets[network::socket::server] = ip6_socket(PORT_SERVER);
    }

    if (!_ip6_sockets[network::socket::client]) {
        if (!(_ip6_sockets[network::socket::client] = ip6_socket(PORT_CLIENT))) {
            _ip6_sockets[network::socket::client] = ip6_socket(0);
        }
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::ip6_socket (int port)
{
    sockaddr_in6 address;
    int newsocket;
    unsigned long args = 1;

    // get socket
    if ( (newsocket = ::socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        return 0;
    }

    // disable blocking
    if (ioctlsocket(newsocket, FIONBIO, &args) == SOCKET_ERROR) {
        return 0;
    }

    // enable broadcasting
    {
        ipv6_mreq mreq = {};

        mreq.ipv6mr_multiaddr.u.Word[ 0 ] = htons( 0xff02 );
        mreq.ipv6mr_multiaddr.u.Word[ 7 ] = htons( 0x0001 );
        mreq.ipv6mr_interface = 0;

        //  add membership to link-local multicast group
        if (setsockopt(newsocket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char const*)&mreq, sizeof(mreq)) == SOCKET_ERROR) {
            return 0;
        }
    }

    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons((word)port);

    if (bind(newsocket, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        closesocket(newsocket);
        return 0;
    }

    return newsocket;
}

//------------------------------------------------------------------------------
char const* manager::address_to_string (network::address a)
{
    static char szString[LONG_STRING];

    if ( a.type == network::address_type::loopback )
        fmt( szString, "loopback\0" );
    else if (a.type == network::address_type::ip4 )
        fmt( szString, "%i.%i.%i.%i:%i\0", a.ip4[0], a.ip4[1], a.ip4[2], a.ip4[3], ntohs( a.port ) );
    else if (a.type == network::address_type::ip6 )
        fmt( szString, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", a.ip6[0], a.ip6[1], a.ip6[2], a.ip6[3], a.ip6[4], a.ip6[5], a.ip6[6], a.ip6[7]);

    return szString;
}

//------------------------------------------------------------------------------
bool manager::string_to_sockaddr (char const *addr, sockaddr_storage *sock)
{
    memset(sock, 0, sizeof(*sock));

    if (inet_pton(AF_INET6, addr, sock) == 1) {
        return true;
    }

    if (inet_pton(AF_INET, addr, sock) == 1) {
        return true;
    }

    addrinfo *info_ptr = nullptr;

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(addr, nullptr, nullptr, &info_ptr) == 0) {
        memcpy(sock, info_ptr->ai_addr, info_ptr->ai_addrlen);
        freeaddrinfo(info_ptr);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
bool manager::string_to_address (char const *addr, network::address *net)
{
    sockaddr_storage sadr;
    
    if ( strcmp( addr, "localhost" ) == 0 )
    {
        memset (net, 0, sizeof(*net));
        net->type = network::address_type::loopback;
        return true;
    }

    if (!string_to_sockaddr (addr, &sadr))
        return false;
    
    sockaddr_to_address (&sadr, net);

    return true;
}

//------------------------------------------------------------------------------
void manager::address_to_sockaddr (network::address *net, sockaddr_storage *sock)
{
    memset (sock, 0, sizeof(*sock));
    sockaddr_in6* sa6 = reinterpret_cast<sockaddr_in6*>(sock);

    sa6->sin6_family = AF_INET6;
    sa6->sin6_port = htons(net->port);

    if (net->type == network::address_type::broadcast) {
        sa6->sin6_addr.u.Word[ 0] = htons(0xff02);
        sa6->sin6_addr.u.Word[ 7] = htons(0x0001);
    } else if (net->type == network::address_type::ip4) {
        sa6->sin6_addr.u.Word[ 5] = 0xffff;
        sa6->sin6_addr.u.Byte[12] = net->ip4[0];
        sa6->sin6_addr.u.Byte[13] = net->ip4[1];
        sa6->sin6_addr.u.Byte[14] = net->ip4[2];
        sa6->sin6_addr.u.Byte[15] = net->ip4[3];
    } else if (net->type == network::address_type::ip6) {
        for (size_t ii = 0; ii < net->ip6.size(); ++ii) {
            sa6->sin6_addr.u.Word[ii] = htons(net->ip6[ii]);
        }
    }
}

//------------------------------------------------------------------------------
void manager::sockaddr_to_address (sockaddr_storage *sock, network::address *net)
{
    if (sock->ss_family == AF_INET)
    {
        net->type = network::address_type::ip4;
        *(int *)&net->ip4 = ((struct sockaddr_in *)sock)->sin_addr.s_addr;
        net->port = ((struct sockaddr_in *)sock)->sin_port;
    }
    else if (sock->ss_family == AF_INET6)
    {
        net->type = network::address_type::ip6;
        net->port = ntohs(((sockaddr_in6*)sock)->sin6_port);

        for (size_t ii = 0; ii < net->ip6.size(); ++ii) {
            net->ip6[ii] = ntohs(((sockaddr_in6*)sock)->sin6_addr.u.Word[ii]);
        }
    }
}

//------------------------------------------------------------------------------
int manager::print (network::socket socket, network::address to, char const *string)
{
    byte        msgbuf[MAX_MSGLEN];
    network::message    msg;

    msg.init( msgbuf, MAX_MSGLEN );

    msg.write_long( -1 );    //  connectionless
    msg.write( string, strlen( string ) );

    return send( socket, msg.bytes_written, msgbuf, to );
}

//------------------------------------------------------------------------------
int manager::send (network::socket socket, int length, void const *data, network::address to)
{
    sockaddr_storage address = {};
    int     net_socket;

    if ( to.type == network::address_type::loopback )
        return send_loopback( socket, length, data, to );

    if ( to.type == network::address_type::broadcast )
        net_socket = _ip6_sockets[socket];
    else if ( to.type == network::address_type::ip4 )
        net_socket = _ip4_sockets[socket];
    else if ( to.type == network::address_type::ip6 )
        net_socket = _ip6_sockets[socket];

    if ( !net_socket )
        return ERROR_FAIL;

    address_to_sockaddr( &to, &address );

    int result = sendto( net_socket, (char const*)data, length, 0, (sockaddr*)&address, sizeof(address) );
    if (result == SOCKET_ERROR)
    {
        int err = WSAGetLastError();

        if (err == WSAEWOULDBLOCK)
            return ERROR_NONE;

        if ((err == WSAEADDRNOTAVAIL) && ((to.type == network::address_type::broadcast)))
            return ERROR_NONE;

        return ERROR_FAIL;
    }

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::get (network::socket socket, network::address *remote_address, network::message *message)
{
    int     protocol;
    sockaddr_storage from = {};
    int     fromlen;
    int     net_socket;
    int     ret;

    if ( get_loopback( socket, remote_address, message ) )
        return true;

    for ( protocol = 0 ; protocol < 2 ; protocol++ )
    {
        if ( protocol == 0 )
            net_socket = _ip4_sockets[socket];
        else
            net_socket = _ip6_sockets[socket];

        if ( !net_socket )
            continue;

        fromlen = sizeof( from );
        ret = recvfrom( net_socket, (char *)message->data, message->size, 0, (sockaddr*)&from, &fromlen );

        sockaddr_to_address( &from, remote_address );

        if ( ret == -1 )
        {
            int err = WSAGetLastError( );

            if (err == WSAEWOULDBLOCK)
                continue;
            if (err == WSAEMSGSIZE)
                continue;

            continue;
        }

        if ( ret == message->size )
            continue;

        message->bytes_written = ret;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
int manager::send_loopback (network::socket socket, int length, void const *data, network::address to)
{
    int     i;
    loopback_t  *loop;

    loop = &_loopbacks[socket^1];

    i = loop->send & (MAX_LOOPBACK-1);
    loop->send++;

    memcpy (loop->msgs[i].data, data, length);
    loop->msgs[i].size = length;

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int manager::get_loopback (network::socket socket, network::address *remote_address, network::message *message)
{
    int     i;
    loopback_t  *loop;

    loop = &_loopbacks[socket];

    if (loop->send - loop->get > MAX_LOOPBACK)
        loop->get = loop->send - MAX_LOOPBACK;

    if (loop->get >= loop->send)
        return false;

    i = loop->get & (MAX_LOOPBACK-1);
    loop->get++;

    memcpy (message->data, loop->msgs[i].data, loop->msgs[i].size);
    message->bytes_written = loop->msgs[i].size;

    memset( remote_address, 0, sizeof(network::address) );
    remote_address->type = network::address_type::loopback;

    return true;
}

} // namespace network
