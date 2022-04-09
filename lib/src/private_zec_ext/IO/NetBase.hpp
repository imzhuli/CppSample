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

}
