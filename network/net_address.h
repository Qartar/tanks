// net_address.h
//

#pragma once

#include "cm_shared.h"

#include <array>

////////////////////////////////////////////////////////////////////////////////
namespace network {

enum address_type { loopback, broadcast, ipv4, ipv6 };

//------------------------------------------------------------------------------
class address
{
public:
    address_type type;

    union {
        std::array<byte, 4> ip4;
        std::array<word, 8> ip6;
    };

    unsigned short port;

public:
    address() = default;
    constexpr address(byte const (&ip)[4], word port = 0)
        : type(address_type::ipv4)
        , ip4({ip[0], ip[1], ip[2], ip[3]})
        , port(port)
    {}
    constexpr address(word const (&ip)[8], word port = 0)
        : type(address_type::ipv6)
        , ip6({ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]})
        , port(port)
    {}

    bool operator==(network::address const& other) const;
    bool operator!=(network::address const& other) const { return !(*this == other); }
    bool is_local() const { return (type == network::address_type::loopback); }
};

} // namespace network
