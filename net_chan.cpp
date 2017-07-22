// net_chan.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
channel::channel(int netport)
{
    LARGE_INTEGER counter;

    QueryPerformanceCounter(&counter);

    if (!netport) {
        _netport = counter.QuadPart & 0xffff;
    } else {
        _netport = netport;
    }
}

//------------------------------------------------------------------------------
void channel::setup(network::socket* socket, network::address remote, int netport)
{
    if (netport) {
        _netport = netport;
    }

    _socket = socket;
    _address = remote;

    _last_sent = g_Application->time();
    _last_received = g_Application->time();
}

//------------------------------------------------------------------------------
bool channel::transmit(int length, byte const* data)
{
    network::message_storage netmsg;

    netmsg.write_long(network::channel::prefix);
    netmsg.write_short(_netport);

    // copy the rest over

    netmsg.write(data, length);

    // send it off

    _last_sent = g_Application->time();

    if (_socket->write(_address, netmsg)) {
        reset();
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------------------------------
bool channel::transmit()
{
    int length = bytes_remaining();
    byte const* data = read(length);

    return transmit(length, data);
}

//------------------------------------------------------------------------------
bool channel::process(network::message&)
{
    _last_received = g_Application->time();
    return true;
}

} // namespace network
