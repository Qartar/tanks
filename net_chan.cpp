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

int channel::init (int netport)
{
    LARGE_INTEGER   nCounter;

    QueryPerformanceCounter( &nCounter );

    if ( !netport )
        this->netport = nCounter.QuadPart & 0xffff;
    else
        this->netport = netport;

    return ERROR_NONE;
}

int channel::setup (network::socket socket, network::address remote, int netport)
{
    if ( netport )
        this->netport = netport;

    this->socket = socket;
    this->address = remote;

    this->message.init( this->messagebuf, MAX_MSGLEN );
    this->message.allow_overflow = true;

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

int channel::transmit (int nLength, byte *pData)
{
    static  byte    netmsgbuf[MAX_MSGLEN];
    network::message        netmsg;

    netmsg.init( netmsgbuf, MAX_MSGLEN );

    // write netport if were are client

    netmsg.write_long( 0 );  // trash
    if (socket == network::socket::client)
        netmsg.write_short( netport );

    // copy the rest over

    netmsg.write( pData, nLength );

    // send it off

    last_sent = g_Application->get_time( );

    return pNet->send( socket, netmsg.bytes_written, netmsgbuf, address );
}

/*
===========================================================

Name    :   cNetChannel::Process

Purpose :   processes a received packet

===========================================================
*/

int channel::process (network::message *pMessage)
{
    int netport;

    pMessage->begin( );

    pMessage->read_long( );  // trash
    if (socket == network::socket::server)
        netport = pMessage->read_short( );

    last_received = g_Application->get_time( );

    return ERROR_NONE;
}

} // namespace network
