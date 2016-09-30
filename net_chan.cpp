/*
===============================================================================

Name	:	net_chan.cpp

Purpose	:	network channel

Date	:	04/14/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

/*
===========================================================

Name	:	cNetChannel::Init

Purpose	:

===========================================================
*/

int cNetChannel::Init (int netport)
{
	LARGE_INTEGER	nCounter;

	QueryPerformanceCounter( &nCounter );

	if ( !netport )
		this->netport = nCounter.QuadPart & 0xffff;
	else
		this->netport = netport;

	return ERROR_NONE;
}

int cNetChannel::Setup (netsock_t socket, netadr_t remote, int netport)
{
	if ( netport )
		this->netport = netport;

	this->socket = socket;
	this->address = remote;

	this->message.Init( this->messagebuf, MAX_MSGLEN );
	this->message.bAllowOverflow = true;

	this->last_sent = g_Application->get_time( );
	this->last_received = g_Application->get_time( );

	return ERROR_NONE;
}

/*
===========================================================

Name	:	cNetChannel::Transmit

Purpose	:	transmits a data to a remote destination

===========================================================
*/

int cNetChannel::Transmit (int nLength, byte *pData)
{
	static	byte	netmsgbuf[MAX_MSGLEN];
	netmsg_t		netmsg;

	netmsg.Init( netmsgbuf, MAX_MSGLEN );

	// write netport if were are client

	netmsg.WriteLong( 0 );	// trash
	if (socket == NS_CLIENT)
		netmsg.WriteShort( netport );

	// copy the rest over

	netmsg.Write( pData, nLength );

	// send it off

	last_sent = g_Application->get_time( );

	return pNet->Send( socket, netmsg.nCurSize, netmsgbuf, address );
}

/*
===========================================================

Name	:	cNetChannel::Process

Purpose	:	processes a received packet

===========================================================
*/

int cNetChannel::Process (netmsg_t *pMessage)
{
	int	netport;

	pMessage->Begin( );

	pMessage->ReadLong( );	// trash
	if (socket == NS_SERVER)
		netport = pMessage->ReadShort( );

	last_received = g_Application->get_time( );

	return ERROR_NONE;
}
