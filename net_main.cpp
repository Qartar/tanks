// net_main.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "net_main.h"

////////////////////////////////////////////////////////////////////////////////
namespace network {

//------------------------------------------------------------------------------
bool address::operator==(network::address const& other) const
{
    if (type != other.type) {
        return false;
    }

    switch (type) {
        case network::address_type::loopback:
            return true;

        case network::address_type::ipv4:
            return ip4 == other.ip4 && port == other.port;

        case network::address_type::ipv6:
            return ip6 == other.ip6 && port == other.port;

        default:
            return false;
    }
}

} // namespace network
