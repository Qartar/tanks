// net_channel.h
//

#pragma once

#include "net_address.h"
#include "net_message.h"

////////////////////////////////////////////////////////////////////////////////
namespace network {

class socket;

//------------------------------------------------------------------------------
class channel : public message_storage
{
public:
    constexpr static int prefix = -1;

public:
    channel(int netport = 0);

    //! set up channel with the remote address over the given network socket
    void setup(network::socket* socket, network::address remote, int netport = 0);

    //! transmit accumulated message to remote address
    bool transmit();

    //! process incoming message
    bool process(network::message& message);

    //! remote address
    network::address const& address() const { return _address; }

    //! network port for address translation
    word netport() const { return _netport; }

    //! time of most recently transmitted message
    time_value last_send() const { return _last_sent; }

    //! time of most recently processed message
    time_value last_received() const { return _last_received; }

protected:
    network::address _address; //!< remote address
    word _netport; //!< port translation

    time_value _last_sent; //!< time of most recently transmitted message
    time_value _last_received; //!< time of most recently processed message

    network::socket* _socket; //!< socket used for transmitting data

protected:
    bool transmit(std::size_t length, byte const* data);
};

} // namespace network
