#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

    struct xNetAddress
    {
        enum : uint8_t {
            eUnknown, eIpv4, eIpv6
        } Type = eUnknown;

        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
        };
        uint16_t Port = 0;

        ZEC_INLINE bool IsV4() const { return Type == eIpv4; }
        ZEC_INLINE bool IsV6() const { return Type == eIpv6; }
        ZEC_INLINE operator bool () const { return Type != eUnknown; }
        ZEC_API_MEMBER std::string ToString() const;

        ZEC_API_STATIC_MEMBER xNetAddress Make(const char * IpStr,   uint16_t Port = 0);
        ZEC_API_STATIC_MEMBER xNetAddress MakeV4(const char * IpStr, uint16_t Port = 0);
        ZEC_API_STATIC_MEMBER xNetAddress MakeV6(const char * IpStr, uint16_t Port = 0);
    };


}