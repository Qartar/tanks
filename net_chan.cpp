/*
===============================================================================

Name    :   net_chan.cpp

Purpose :   network channel

Date    :   04/14/2004

===============================================================================
*/

#include "local.h"
#pragma hdrstop

namespace network {

/*
===========================================================

Name    :   cNetChannel::Init

Purpose :

===========================================================
*/

int channel::Init (int netport)
{
    LARGE_INTEGER   nCounter;

    QueryPerformanceCounter( &nCounter );

    if ( !netport )
        this->netport = nCounter.QuadPart & 0xffff;
    else
        this->netport = netport;

    return ERROR_NONE;
}

int channel::Setup (network::socket socket, network::address remote, int netport)
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

Name    :   cNetChannel::Transmit

Purpose :   transmits a data to a remote destination

===========================================================
*/

int channel::Transmit (int nLength, byte *pData)
{
    static  byte    netmsgbuf[MAX_MSGLEN];
    network::message        netmsg;

    netmsg.Init( netmsgbuf, MAX_MSGLEN );

    // write netport if were are client

    netmsg.WriteLong( 0 );  // trash
    if (socket == network::socket::client)
        netmsg.WriteShort( netport );

    // copy the rest over

    netmsg.Write( pData, nLength );

    // send it off

    last_sent = g_Application->get_time( );

    return pNet->Send( socket, netmsg.nCurSize, netmsgbuf, address );
}

/*
===========================================================

Name    :   cNetChannel::Process

Purpose :   processes a received packet

===========================================================
*/

int channel::Process (network::message *pMessage)
{
    int netport;

    pMessage->Begin( );

    pMessage->ReadLong( );  // trash
    if (socket == network::socket::server)
        netport = pMessage->ReadShort( );

    last_received = g_Application->get_time( );

    return ERROR_NONE;
}

} // namespace network
