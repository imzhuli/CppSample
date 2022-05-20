#pragma once
#include "./_Local.hpp"
#include <zec_ext/IO/NetBase.hpp>

ZEC_NS
{

    ZEC_STATIC_INLINE xNetAddress MakeNetAddress(const tcp::endpoint & NativeEndpoint) {
        xNetAddress Ret;
        auto Address = NativeEndpoint.address();
        if (Address.is_v4()) {
            Ret.Type = xNetAddress::eIpv4;
            auto AddrBytes = Address.to_v4().to_bytes();
            memcpy(Ret.Ipv4, AddrBytes.data(), sizeof(Ret.Ipv4));
        } else if (Address.is_v6()) {
            Ret.Type = xNetAddress::eIpv6;
            auto AddrBytes = Address.to_v6().to_bytes();
            memcpy(Ret.Ipv6, AddrBytes.data(), sizeof(Ret.Ipv6));
        }
        Ret.Port = NativeEndpoint.port();
        return Ret;
    }

    ZEC_STATIC_INLINE xNetAddress MakeNetAddress(const udp::endpoint & NativeEndpoint) {
        xNetAddress Ret;
        auto Address = NativeEndpoint.address();
        if (Address.is_v4()) {
            Ret.Type = xNetAddress::eIpv4;
            auto AddrBytes = Address.to_v4().to_bytes();
            memcpy(Ret.Ipv4, AddrBytes.data(), sizeof(Ret.Ipv4));
        } else if (Address.is_v6()) {
            Ret.Type = xNetAddress::eIpv6;
            auto AddrBytes = Address.to_v6().to_bytes();
            memcpy(Ret.Ipv6, AddrBytes.data(), sizeof(Ret.Ipv6));
        }
        Ret.Port = NativeEndpoint.port();
        return Ret;
    }

    ZEC_STATIC_INLINE xTcpEndpoint MakeTcpEndpoint(const xNetAddress & Address) {
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
