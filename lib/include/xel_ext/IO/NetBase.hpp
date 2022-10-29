#pragma once
#include <xel/Common.hpp>
#include <string>
#include <array>
#include <cstring>

X_NS
{

    struct xNetAddress
    {
        enum : uint16_t {
            eUnknown, eIpv4, eIpv6
        } Type = eUnknown;

        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
            ubyte IpStorage[16] = {};
        };
        uint16_t Port = 0;

        using xKeyType = std::array<ubyte, 20>;

        X_INLINE bool IsV4() const { return Type == eIpv4; }
        X_INLINE bool IsV6() const { return Type == eIpv6; }
        X_INLINE operator bool () const { return Type != eUnknown; }
        X_INLINE xKeyType AsKey() const {
            xKeyType Ret;
            memcpy(Ret.data(), this, sizeof(*this));
            return Ret;
        }
        X_INLINE bool operator == (const xNetAddress & Other) const {
            if (Type == eUnknown || Type != Other.Type || Port != Other.Port) {
                return false;
            }
            if (Type == eIpv4) {
                return !memcmp(Ipv4, Other.Ipv4, sizeof(Ipv4));
            }
            if (Type == eIpv6) {
                return !memcmp(Ipv6, Other.Ipv6, sizeof(Ipv6));
            }
            return false;
        }

        X_API_MEMBER std::string IpToString() const;
        X_API_MEMBER std::string ToString() const;

        X_API_STATIC_MEMBER xNetAddress Make(const char * IpStr,   uint16_t Port = 0);
        X_API_STATIC_MEMBER xNetAddress MakeV4Raw(const void * AddrRaw, uint16_t Port = 0);
        X_API_STATIC_MEMBER xNetAddress MakeV4(const char * IpStr, uint16_t Port = 0);
        X_API_STATIC_MEMBER xNetAddress MakeV6Raw(const void * AddrRaw, uint16_t Port = 0);
        X_API_STATIC_MEMBER xNetAddress MakeV6(const char * IpStr, uint16_t Port = 0);
        X_API_STATIC_MEMBER xNetAddress Parse(const std::string & AddressStr);
    };

}
