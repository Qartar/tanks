/*
===============================================================================

Name    :   net_main.h

Purpose :   network communication

Date    :   03/31/2005

===============================================================================
*/

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

typedef enum netadrtype_s {NA_LOOPBACK, NA_BROADCAST, NA_IP, NA_IPX, NA_BROADCAST_IPX} netadrtype_t;
typedef enum netsock_s { NS_CLIENT, NS_SERVER, NUM_SOCKETS } netsock_t;

typedef class cNetAddress
{
public:
    bool operator == (cNetAddress &other);
    bool operator != (cNetAddress &other) { return !(*this == other); }
    bool isLocal() { return (type == NA_LOOPBACK); }

    netadrtype_t    type;

    byte    ip[4];
    byte    ipx[10];

    unsigned short      port;
} netadr_t;

typedef class cNetMessage
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
} netmsg_t;

typedef class cNetChannel
{
public:
    int Init (int netport = 0);
    int Setup (netsock_t socket, netadr_t remote, int netport = 0);

    int Transmit (int nLength, byte *pData);
    int Process (netmsg_t *pMessage);

    netsock_t   socket;

    netadr_t    address;    //  remote address
    int         netport;    //  port translation

    netmsg_t    message;    // outgoing
    byte        messagebuf[MAX_MSGLEN];

    float       last_sent;
    float       last_received;

    float       connect_time;
    int         connect_tries;
} netchan_t;

/*
===========================================================

    Internal Components

===========================================================
*/

typedef struct loopmsg_s
{
    byte    data[MAX_MSGLEN];
    int size;
} loopmsg_t;

#define MAX_LOOPBACK    4

typedef struct loopback_s
{
    loopmsg_t   msgs[MAX_LOOPBACK];
    int     get, send;
} loopback_t;

/*
===========================================================

Name    :   cNetwork

===========================================================
*/

class cNetwork
{
public:
    friend  cNetChannel;

    int Init ();
    int Shutdown ();

    int Config (bool multiplayer);

    int Print (netsock_t socket, netadr_t to, char *szMessage);

    int Get (netsock_t socket, netadr_t *pFrom, netmsg_t *pMessage);

    bool    StringToNet (char *addr, netadr_t *net);
    char    *NetToString (netadr_t a);
private:
    int Send (netsock_t socket, int nLength, void *pData, netadr_t to);

    int GetLoop (netsock_t socket, netadr_t *pFrom, netmsg_t *pMessage);
    int SendLoop (netsock_t socket, int nLength, void *pData, netadr_t to);

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

    void        NetToSock (netadr_t *net, sockaddr *sock);
    void        SockToNet (sockaddr *sock, netadr_t *net);
    bool        StringToSock (char *addr, sockaddr *sock);

};

extern cNetwork *pNet;
