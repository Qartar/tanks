// net_chan.cpp
//

#include "local.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
int channel::setup (network::socket socket, network::address remote, int netport)
{
    if ( netport )
        this->netport = netport;

    this->socket = socket;
    this->address = remote;

    this->message.init( this->messagebuf, MAX_MSGLEN );
    this->message.allow_overflow = true;

    this->last_sent = g_Application->time();
    this->last_received = g_Application->time();

    return ERROR_NONE;
}

//------------------------------------------------------------------------------
int channel::transmit (int length, byte *data)
{
    static  byte    netmsgbuf[MAX_MSGLEN];
    network::message        netmsg;

    netmsg.init( netmsgbuf, MAX_MSGLEN );

    // write netport if were are client

    netmsg.write_long( 0 );  // trash
    if (socket == network::socket::client)
        netmsg.write_short( netport );

    // copy the rest over

    netmsg.write( data, length );

    // send it off

    last_sent = g_Application->time();

    return pNet->send( socket, netmsg.bytes_written, netmsgbuf, address );
}

//------------------------------------------------------------------------------
int channel::process (network::message *message)
{
    int netport;

    message->begin( );

    message->read_long( );  // trash
    if (socket == network::socket::server)
        netport = message->read_short( );

    last_received = g_Application->time();

    return ERROR_NONE;
}

} // namespace network
