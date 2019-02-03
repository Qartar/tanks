// net_chan.cpp
//

#include "net_channel.h"
#include "net_socket.h"

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
channel::channel(word netport)
    : _address{}
    , _last_sent{}
    , _last_received{}
    , _socket{}
{
    if (!netport) {
        _netport = time_value::current().to_microseconds() & 0xffff;
    } else {
        _netport = netport;
    }
}

//------------------------------------------------------------------------------
void channel::setup(network::socket* socket, network::address remote, word netport)
{
    if (netport) {
        _netport = netport;
    }

    _socket = socket;
    _address = remote;

    _last_sent = time_value::current();
    _last_received = time_value::current();
}

//------------------------------------------------------------------------------
bool channel::transmit(std::size_t length, byte const* data)
{
    network::message_storage netmsg;

    netmsg.write_long(network::channel::prefix);
    netmsg.write_short(_netport);

    // copy the rest over

    netmsg.write(data, length);

    // send it off

    _last_sent = time_value::current();

    if (_socket && _socket->write(_address, netmsg)) {
        reset();
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------------------------------
bool channel::transmit()
{
    std::size_t length = bytes_remaining();
    byte const* data = read(length);

    return transmit(length, data);
}

//------------------------------------------------------------------------------
bool channel::process(network::message&)
{
    _last_received = time_value::current();
    return true;
}

} // namespace network
