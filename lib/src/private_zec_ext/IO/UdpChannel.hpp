#pragma once
#include "./_Local.hpp"
#include <zec/Memory.hpp>
#include <zec_ext/IO/UdpChannel.hpp>

ZEC_NS
{
    using xUdpSocket = udp::socket;
    using xUdpEndpoint = udp::endpoint;

    ZEC_STATIC_INLINE xUdpEndpoint MakeUdpEndpoint(const xNetAddress & Address) {
        if (Address.IsV4()) {
            static_assert(std::tuple_size<ip::address_v4::bytes_type>() == 4);
            ip::address_v4::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv4, 4);
            return { ip::make_address_v4(Bytes), Address.Port };
        } else if (Address.IsV6()) {
            static_assert(std::tuple_size<ip::address_v6::bytes_type>() == 16);
            ip::address_v6::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv6, 16);
            return { ip::make_address_v6(Bytes), Address.Port };
        }
        return {};
    }

}
