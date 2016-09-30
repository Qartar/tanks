/*
===============================================================================

Name	:	net_main.cpp

Purpose	:	network communication

Date	:	04/01/2005

===============================================================================
*/

#include "net_main.h"

cNetwork	*pNet;	// extern

/*
===========================================================

Name	:	cNetAddress::operator==

Purpose	:	compares base address

===========================================================
*/

bool cNetAddress::operator == (cNetAddress &other)
{
	if ( type != other.type )
		return false;

	if ( type == NA_LOOPBACK )
		return true;

	if ( type == NA_IP )
	{
		if ( memcmp( ip, other.ip, 4 ) == 0 )
			return true;
		return false;
	}

	if ( type == NA_IPX )
	{
		if ( memcmp( ipx, other.ipx, 10 ) == 0 )
			return true;
		return false;
	}

	return false;
}

/*
===========================================================

Name	:	cNetwork::Init

Purpose	:	

===========================================================
*/

int cNetwork::Init ()
{
	memset( m_Loopbacks, 0, sizeof(loopback_t)*NUM_SOCKETS );

	if ( WSAStartup( MAKEWORD( 1, 1 ), &m_wsadata ) )
		return ERROR_FAIL;

	Config( false );

	return ERROR_NONE;
}

int cNetwork::Shutdown ()
{
	Config( false );

	WSACleanup( );

	return ERROR_NONE;
}

int cNetwork::Config (bool multiplayer)
{
	if ( multiplayer == m_multiplayer )
		return ERROR_NONE;

	if ( m_multiplayer = multiplayer )
	{
		OpenIP( );
		OpenIPX( );
	}
	else
	{
		int		i;

		// shut it down
		for (i=0 ; i<NUM_SOCKETS ; i++)
		{
			if ( ip_sockets[i] )
			{
				closesocket( ip_sockets[i] );
				ip_sockets[i] = 0;
			}

			if ( ipx_sockets[i] )
			{
				closesocket( ipx_sockets[i] );
				ipx_sockets[i] = 0;
			}
		}
	}


	return ERROR_NONE;
}

char *cNetwork::WSAErrorString (int code)
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

/*
===============================================================================

Name	:	IP

===============================================================================
*/

#define PORT_ANY	-1

int cNetwork::OpenIP ()
{
	int	port;

	if ( !ip_sockets[NS_SERVER] )
	{
		port = PORT_SERVER;

		ip_sockets[NS_SERVER] = IP_Socket( port );
	}

	if ( !ip_sockets[NS_CLIENT] )
	{
		port = PORT_CLIENT;

		if ( !(ip_sockets[NS_CLIENT] = IP_Socket( port )) )
			ip_sockets[NS_CLIENT] = IP_Socket( PORT_ANY );
	}

	return ERROR_NONE;
}

int cNetwork::IP_Socket (int port)
{
	sockaddr_in		address;
	int		newsocket;
	unsigned long		args;

	// get socket
	if ( (newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		return 0;

	// disable blocking
	if (ioctlsocket (newsocket, FIONBIO, &args) == -1)
		return 0;

	// enable broadcasting
	if (setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&args, sizeof(args)) == -1)
		return 0;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	if ( port == PORT_ANY )
		address.sin_port = 0;
	else
		address.sin_port = htons( (unsigned short )port );

	if (bind (newsocket, (sockaddr *)&address, sizeof(address)) == -1)
	{
		closesocket( newsocket );
		return 0;
	}

	return newsocket;
}

/*
===============================================================================

Name	:	IPX

===============================================================================
*/

int cNetwork::OpenIPX ()
{
	return ERROR_NONE;
}

int cNetwork::IPX_Socket (int port)
{
	return 0;
}

/*
===============================================================================

Name	:	address conversions

===============================================================================
*/

char *cNetwork::NetToString (netadr_t a)
{
	static char	szString[LONG_STRING];

	if ( a.type == NA_LOOPBACK )
		fmt( szString, "loopback\0" );
	else if (a.type == NA_IP )
		fmt( szString, "%i.%i.%i.%i:%i\0", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs( a.port ) );
	else if ( a.type == NA_IPX )
		fmt( szString, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i\0", a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], ntohs(a.port));

	return szString;
}

#define DO(src,dest)	\
	copy[0] = addr[src];	\
	copy[1] = addr[src + 1];	\
	sscanf (copy, "%x", &val);	\
	((struct sockaddr_ipx *)sock)->dest = val

bool cNetwork::StringToSock (char *addr, sockaddr *sock)
{
	struct hostent	*h;
	char	*colon;
	int		val;
	char	copy[128];
	
	memset (sock, 0, sizeof(*sock));

	if ((strlen(addr) >= 23) && (addr[8] == ':') && (addr[21] == ':'))	// check for an IPX address
	{
		((struct sockaddr_ipx *)sock)->sa_family = AF_IPX;
		copy[2] = 0;
		DO(0, sa_netnum[0]);
		DO(2, sa_netnum[1]);
		DO(4, sa_netnum[2]);
		DO(6, sa_netnum[3]);
		DO(9, sa_nodenum[0]);
		DO(11, sa_nodenum[1]);
		DO(13, sa_nodenum[2]);
		DO(15, sa_nodenum[3]);
		DO(17, sa_nodenum[4]);
		DO(19, sa_nodenum[5]);
		sscanf (&addr[22], "%u", &val);
		((struct sockaddr_ipx *)sock)->sa_socket = htons((unsigned short)val);
	}
	else
	{
		((struct sockaddr_in *)sock)->sin_family = AF_INET;
		
		((struct sockaddr_in *)sock)->sin_port = 0;

		strcpy (copy, addr);
		// strip off a trailing :port if present
		for (colon = copy ; *colon ; colon++)
			if (*colon == ':')
			{
				*colon = 0;
				((struct sockaddr_in *)sock)->sin_port = htons((short)atoi(colon+1));	
			}
		
		if (copy[0] >= '0' && copy[0] <= '9')
		{
			*(int *)&((struct sockaddr_in *)sock)->sin_addr = inet_addr(copy);
		}
		else
		{
			if (! (h = gethostbyname(copy)) )
				return 0;
			*(int *)&((struct sockaddr_in *)sock)->sin_addr = *(int *)h->h_addr_list[0];
		}
	}
	
	return true;
}

#undef DO

bool cNetwork::StringToNet (char *addr, netadr_t *net)
{
	sockaddr	sadr;
	
	if ( strcmp( addr, "localhost" ) == 0 )
	{
		memset (net, 0, sizeof(*net));
		net->type = NA_LOOPBACK;
		return true;
	}

	if (!StringToSock (addr, &sadr))
		return false;
	
	SockToNet (&sadr, net);

	return true;
}

void cNetwork::NetToSock (netadr_t *net, sockaddr *sock)
{
	memset (sock, 0, sizeof(*sock));

	if (net->type == NA_BROADCAST)
	{
		((struct sockaddr_in *)sock)->sin_family = AF_INET;
		((struct sockaddr_in *)sock)->sin_port = net->port;
		((struct sockaddr_in *)sock)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if (net->type == NA_IP)
	{
		((struct sockaddr_in *)sock)->sin_family = AF_INET;
		((struct sockaddr_in *)sock)->sin_addr.s_addr = *(int *)&net->ip;
		((struct sockaddr_in *)sock)->sin_port = net->port;
	}
	else if (net->type == NA_IPX)
	{
		((struct sockaddr_ipx *)sock)->sa_family = AF_IPX;
		memcpy(((struct sockaddr_ipx *)sock)->sa_netnum, &net->ipx[0], 4);
		memcpy(((struct sockaddr_ipx *)sock)->sa_nodenum, &net->ipx[4], 6);
		((struct sockaddr_ipx *)sock)->sa_socket = net->port;
	}
	else if (net->type == NA_BROADCAST_IPX)
	{
		((struct sockaddr_ipx *)sock)->sa_family = AF_IPX;
		memset(((struct sockaddr_ipx *)sock)->sa_netnum, 0, 4);
		memset(((struct sockaddr_ipx *)sock)->sa_nodenum, 0xff, 6);
		((struct sockaddr_ipx *)sock)->sa_socket = net->port;
	}
}

void cNetwork::SockToNet (sockaddr *sock, netadr_t *net)
{
	if (sock->sa_family == AF_INET)
	{
		net->type = NA_IP;
		*(int *)&net->ip = ((struct sockaddr_in *)sock)->sin_addr.s_addr;
		net->port = ((struct sockaddr_in *)sock)->sin_port;
	}
	else if (sock->sa_family == AF_IPX)
	{
		net->type = NA_IPX;
		memcpy(&net->ipx[0], ((struct sockaddr_ipx *)sock)->sa_netnum, 4);
		memcpy(&net->ipx[4], ((struct sockaddr_ipx *)sock)->sa_nodenum, 6);
		net->port = ((struct sockaddr_ipx *)sock)->sa_socket;
	}
}

/*
===========================================================

Name	:	cNetwork::Print

Purpose	:	sends a connectionless packet

===========================================================
*/

int cNetwork::Print (netsock_t socket, netadr_t to, char *szMessage)
{
	byte		msgbuf[MAX_MSGLEN];
	netmsg_t	msg;

	msg.Init( msgbuf, MAX_MSGLEN );

	msg.WriteLong( -1 );	//	connectionless
	msg.Write( szMessage, strlen( szMessage ) );

	return Send( socket, msg.nCurSize, msgbuf, to );
}

/*
===========================================================

Name	:	cNetwork::Send

Purpose	:	sends a packet across the network

===========================================================
*/

int cNetwork::Send (netsock_t socket, int nLength, void *pData, netadr_t to)
{
	sockaddr	address;
	int		net_socket;

	if ( to.type == NA_LOOPBACK )
		return SendLoop( socket, nLength, pData, to );

	if ( to.type == NA_BROADCAST )
		net_socket = ip_sockets[socket];
	else if ( to.type == NA_IP )
		net_socket = ip_sockets[socket];
	else if ( to.type == NA_IPX )
		net_socket = ip_sockets[socket];
	else if ( to.type == NA_BROADCAST_IPX )
		net_socket = ipx_sockets[socket];

	if ( !net_socket )
		return ERROR_FAIL;

	NetToSock( &to, &address );

	if ( (sendto( net_socket, (char *)pData, nLength, 0, &address, sizeof(address) )) == -1 )
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return ERROR_NONE;

		if ((err == WSAEADDRNOTAVAIL) && ((to.type == NA_BROADCAST) || (to.type == NA_BROADCAST_IPX)))
			return ERROR_NONE;

		return ERROR_FAIL;
	}

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cNetwork::Get

Purpose	:	gets any packets queued

===========================================================
*/

int cNetwork::Get (netsock_t socket, netadr_t *pFrom, netmsg_t *pMessage)
{
	int		protocol;
	sockaddr	from;
	int		fromlen;
	int		net_socket;
	int		ret;

	if ( GetLoop( socket, pFrom, pMessage ) )
		return true;

	for ( protocol = 0 ; protocol < 2 ; protocol++ )
	{
		if ( protocol == 0 )
			net_socket = ip_sockets[socket];
		else
			net_socket = ip_sockets[socket];

		if ( !net_socket )
			continue;

		fromlen = sizeof( from );
		ret = recvfrom( net_socket, (char *)pMessage->pData, pMessage->nMaxSize, 0, &from, &fromlen );

		SockToNet( &from, pFrom );

		if ( ret == -1 )
		{
			int err = WSAGetLastError( );

			if (err == WSAEWOULDBLOCK)
				continue;
			if (err == WSAEMSGSIZE)
				continue;

			continue;
		}

		if ( ret == pMessage->nMaxSize )
			continue;

		pMessage->nCurSize = ret;
		return true;
	}

	return false;
}

/*
===========================================================

Name	:	cNetwork::SendLoop

Purpose	:	sends a packet to loopback

===========================================================
*/

int cNetwork::SendLoop (netsock_t socket, int nLength, void *pData, netadr_t to)
{
	int		i;
	loopback_t	*loop;

	loop = &m_Loopbacks[socket^1];

	i = loop->send & (MAX_LOOPBACK-1);
	loop->send++;

	memcpy (loop->msgs[i].data, pData, nLength);
	loop->msgs[i].size = nLength;

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cNetwork::GetLoop

Purpose	:	gets a packet from loopback

===========================================================
*/

int cNetwork::GetLoop (netsock_t socket, netadr_t *pFrom, netmsg_t *pMessage)
{
	int		i;
	loopback_t	*loop;

	loop = &m_Loopbacks[socket];

	if (loop->send - loop->get > MAX_LOOPBACK)
		loop->get = loop->send - MAX_LOOPBACK;

	if (loop->get >= loop->send)
		return false;

	i = loop->get & (MAX_LOOPBACK-1);
	loop->get++;

	memcpy (pMessage->pData, loop->msgs[i].data, loop->msgs[i].size);
	pMessage->nCurSize = loop->msgs[i].size;

	memset( pFrom, 0, sizeof(netadr_t) );
	pFrom->type = NA_LOOPBACK;

	return true;
}
