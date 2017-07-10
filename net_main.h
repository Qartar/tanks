// net_main.h
//

#pragma once

#include "shared.h"

#include <winsock.h>
#include <wsipx.h>

/*
===========================================================

    External Components

===========================================================
*/

#define MAX_MSGLEN  1400

////////////////////////////////////////////////////////////////////////////////
namespace network {

enum address_type {loopback, broadcast, ip, ipx, broadcast_ipx};
enum socket { client, server, NUM_SOCKETS };

//------------------------------------------------------------------------------
class address
{
public:
    bool operator == (network::address &other);
    bool operator != (network::address &other) { return !(*this == other); }
    bool isLocal() { return (type == network::address_type::loopback); }

    address_type    type;

    byte    ip[4];
    byte    ipx[10];

    unsigned short      port;
};

//------------------------------------------------------------------------------
class message
{
public:
    byte    *pData;
    int     nMaxSize;
    int     nCurSize;
    int     nRemaining;
    int     nReadCount;
    bool    bAllowOverflow;
    bool    bOverflowed;

    void    *Alloc (int nSize);

    void    Init (byte *pData, int nMaxSize);
    void    Clear ();
    void    Write (void const* pData, int nSize);
    int Read (void *pOut, int nSize);

    void    WriteByte (int b);
    void    WriteShort (int s);
    void    WriteLong (int l);
    void    WriteFloat (float f);
    void    WriteChar (int c);
    void    WriteString (char const* sz);
    void    WriteAngle (float f);
    void    WriteVector (vec2 v);

    void    Begin ();

    int ReadByte ();
    int ReadShort ();
    int ReadLong ();
    float   ReadFloat ();
    int ReadChar ();
    char    *ReadString ();
    char    *ReadLine ();
    float   ReadAngle ();
    vec2    ReadVector ();
};

//------------------------------------------------------------------------------
class channel
{
public:
    int Init (int netport = 0);
    int Setup (network::socket socket, network::address remote, int netport = 0);

    int Transmit (int nLength, byte *pData);
    int Process (network::message *pMessage);

    network::socket   socket;

    network::address    address;    //  remote address
    int         netport;    //  port translation

    network::message    message;    // outgoing
    byte        messagebuf[MAX_MSGLEN];

    float       last_sent;
    float       last_received;

    float       connect_time;
    int         connect_tries;
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
    friend  channel;

    int Init ();
    int Shutdown ();

    int Config (bool multiplayer);

    int Print (network::socket socket, network::address to, char *szMessage);

    int Get (network::socket socket, network::address *pFrom, network::message *pMessage);

    bool    StringToNet (char *addr, network::address *net);
    char    *NetToString (network::address a);
private:
    int Send (network::socket socket, int nLength, void *pData, network::address to);

    int GetLoop (network::socket socket, network::address *pFrom, network::message *pMessage);
    int SendLoop (network::socket socket, int nLength, void *pData, network::address to);

    loopback_t  m_Loopbacks[NUM_SOCKETS];

    bool        m_multiplayer;

    // winsock

    WSADATA     m_wsadata;

    char        *WSAErrorString (int code = 0);

    int     ip_sockets[NUM_SOCKETS];
    int     ipx_sockets[NUM_SOCKETS];

    int     OpenIP ();
    int     OpenIPX ();

    int     IP_Socket (int port);
    int     IPX_Socket (int port);

    void        NetToSock (network::address *net, sockaddr *sock);
    void        SockToNet (sockaddr *sock, network::address *net);
    bool        StringToSock (char *addr, sockaddr *sock);

};

} // namespace network

extern network::manager *pNet;
