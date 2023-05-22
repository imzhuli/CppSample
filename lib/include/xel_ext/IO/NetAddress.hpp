#pragma once
#include "./_.hpp"
#include <string>
#include <array>
#include <cstring>

X_NS
{

    struct xNetAddress final
    {
        using xKeyType = std::array<ubyte, 20>;
        enum : uint16_t {
            eUnknown, eIpv4, eIpv6
        } Type = eUnknown;
        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
            ubyte IpStorage[16] = {};
        };
        uint16_t Port = 0;

        // methods:
        X_INLINE bool IsV4() const { return Type == eIpv4; }
        X_INLINE bool IsV6() const { return Type == eIpv6; }
        X_INLINE operator bool () const { return Type != eUnknown; }
        X_INLINE xKeyType AsKey() const {
            xKeyType Ret;
            memcpy(Ret.data(), this, sizeof(*this));
            return Ret;
        }

        X_INLINE bool operator == (const xNetAddress & Other) const {
            if (Type != Other.Type || Port != Other.Port) {
                return false;
            }
            if (Type == eIpv4) {
                return !memcmp(Ipv4, Other.Ipv4, sizeof(Ipv4));
            }
            if (Type == eIpv6) {
                return !memcmp(Ipv6, Other.Ipv6, sizeof(Ipv6));
            }
            return true; // both of type unknown
        }

        X_INLINE bool operator != (const xNetAddress & Other) const {
            return ! (*this == Other);
        }

        X_INLINE int GetAddressFamily() const {
            if (Type == eIpv4) {
                return AF_INET;
            }
            if (Type == eIpv6) {
                return AF_INET6;
            }
            return AF_UNSPEC;
        }

        X_INLINE void Dump(sockaddr_in * Addr4Ptr) const {
            assert(IsV4());
            memset(Addr4Ptr, 0, sizeof(*Addr4Ptr));
            auto & Addr4 = *Addr4Ptr;
            Addr4.sin_family = AF_INET;
            Addr4.sin_addr = (decltype(sockaddr_in::sin_addr)&)(Ipv4);
            Addr4.sin_port = htons(Port);
        }

        X_INLINE void Dump(sockaddr_in6 * Addr6Ptr) const {
            assert(IsV6());
            memset(Addr6Ptr, 0, sizeof(*Addr6Ptr));
            auto & Addr6 = *Addr6Ptr;
            Addr6.sin6_family = AF_INET6;
            Addr6.sin6_addr = (decltype(sockaddr_in6::sin6_addr)&)(Ipv6);
            Addr6.sin6_port = htons(Port);
        }

        X_INLINE size_t Dump(sockaddr_storage * AddrStoragePtr) const {
            if (IsV4()) {
                Dump((sockaddr_in *)AddrStoragePtr);
                return sizeof(sockaddr_in);
            }
            if (IsV6()) {
                Dump((sockaddr_in6 *)AddrStoragePtr);
                return sizeof(sockaddr_in6);
            }
            *AddrStoragePtr = sockaddr_storage{};
            AddrStoragePtr->ss_family = AF_UNSPEC;
            return 0;
        }

        X_API_MEMBER std::string IpToString() const;
        X_API_MEMBER std::string ToString() const;

        X_STATIC_INLINE xNetAddress Make4() { return xNetAddress { .Type = eIpv4 }; }
        X_STATIC_INLINE xNetAddress Make6() { return xNetAddress { .Type = eIpv6 }; }

        X_STATIC_INLINE xNetAddress Make4Raw(const void * RawPtr, uint16_t Port) {
            auto Address = xNetAddress { .Type = eIpv4, .Port = Port };
            memcpy(Address.Ipv4, RawPtr, sizeof(Address.Ipv4));
            return Address;
        }
        X_STATIC_INLINE xNetAddress Make6Raw(const void * RawPtr, uint16_t Port) {
            auto Address = xNetAddress { .Type = eIpv6, .Port = Port };
            memcpy(Address.Ipv6, RawPtr, sizeof(Address.Ipv6));
            return Address;
        }

        X_API_STATIC_MEMBER xNetAddress Parse(const char * IpStr, uint16_t Port);
        X_API_STATIC_MEMBER xNetAddress Parse(const std::string & AddressStr);
        X_API_STATIC_MEMBER xNetAddress Parse(const struct sockaddr * SockAddrPtr);

        X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr_in * SockAddr4Ptr);
        X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr_in6 * SockAddr6Ptr);
    };

}
