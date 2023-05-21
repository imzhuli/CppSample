#include <xel_ext/IO/NetAddress.hpp>
#include <xel/String.hpp>

X_NS
{

    xNetAddress xNetAddress::Parse(const char * IpStr, uint16_t Port)
    {
        xNetAddress Ret;
        auto parse4 = inet_pton(AF_INET, IpStr, Ret.Ipv4);
        if (1 == parse4) {
            Ret.Type = xNetAddress::eIpv4;
            Ret.Port = Port;
            return Ret;
        }
        auto parse6 = inet_pton(AF_INET6, IpStr, Ret.Ipv6);
        if (1 == parse6) {
            Ret.Type = xNetAddress::eIpv6;
            Ret.Port = Port;
            return Ret;
        }
        return Ret;
    }

    xNetAddress xNetAddress::Parse(const std::string & AddressStr)
    {
        uint16_t Port = 0;
        auto Segs = Split(AddressStr, ":");
        if (Segs.size() > 2) {
            return {};
        }
        if (Segs.size() == 2) {
            Port = (uint16_t)atoll(Segs[1].c_str());
        }
        auto IpStr = Segs[0].c_str();
        return Parse(IpStr, Port);
    }

    xNetAddress xNetAddress::Parse(const struct sockaddr * SockAddrPtr)
    {
        if (SockAddrPtr->sa_family == AF_INET) {
            auto Addr4Ptr = (const sockaddr_in * )SockAddrPtr;
            xNetAddress Result = {};
            Result.Type = xNetAddress::eIpv4;
            Result.Port = ntohs(Addr4Ptr->sin_port);
            memcpy(Result.Ipv4, &Addr4Ptr->sin_addr, sizeof(Result.Ipv4));
        }
        else if (SockAddrPtr->sa_family == AF_INET6) {
            auto Addr6Ptr = (const sockaddr_in6 * )SockAddrPtr;
            xNetAddress Result = {};
            Result.Type = xNetAddress::eIpv6;
            Result.Port = ntohs(Addr6Ptr->sin6_port);
            memcpy(Result.Ipv6, &Addr6Ptr->sin6_addr, sizeof(Result.Ipv6));
        }
        return {};
    }

    xNetAddress xNetAddress::Parse(const sockaddr_in * SockAddr4Ptr)
    {
        assert(SockAddr4Ptr->sin_family == AF_INET);
        auto Ret = Make4();
        memcpy(Ret.Ipv4, &SockAddr4Ptr->sin_addr,sizeof(Ret.Ipv4));
        Ret.Port = ntohs(SockAddr4Ptr->sin_port);
        return Ret;
    }

    xNetAddress xNetAddress::Parse(const sockaddr_in6 * SockAddr6Ptr)
    {
        assert(SockAddr6Ptr->sin6_family == AF_INET6);
        auto Ret = Make6();
        memcpy(Ret.Ipv6, &SockAddr6Ptr->sin6_addr,sizeof(Ret.Ipv6));
        Ret.Port = ntohs(SockAddr6Ptr->sin6_port);
        return Ret;
    }

    std::string xNetAddress::IpToString() const
    {
        char Buffer[64];
        if (Type == eUnknown) {
            return "Unknown";
        }
        if (Type == eIpv4) {
            return {Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d",
                (int)Ipv4[0],
                (int)Ipv4[1],
                (int)Ipv4[2],
                (int)Ipv4[3])};
        }
        // ipv6
        return {Buffer, (size_t)sprintf(Buffer,
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x",
            (int)Ipv6[0],(int)Ipv6[1],(int)Ipv6[2],(int)Ipv6[3],
            (int)Ipv6[4],(int)Ipv6[5],(int)Ipv6[6],(int)Ipv6[7],
            (int)Ipv6[8],(int)Ipv6[9],(int)Ipv6[10],(int)Ipv6[11],
            (int)Ipv6[12],(int)Ipv6[13],(int)Ipv6[14],(int)Ipv6[15])};
    }

    std::string xNetAddress::ToString() const
    {
        char Buffer[64];
        if (Type == eUnknown) {
            return "Unknown";
        }
        if (Type == eIpv4) {
            return {Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d:%u",
                (int)Ipv4[0],
                (int)Ipv4[1],
                (int)Ipv4[2],
                (int)Ipv4[3],
                (int)Port)};
        }
        // ipv6
        return {Buffer, (size_t)sprintf(Buffer,
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%u",
            (int)Ipv6[0],(int)Ipv6[1],(int)Ipv6[2],(int)Ipv6[3],
            (int)Ipv6[4],(int)Ipv6[5],(int)Ipv6[6],(int)Ipv6[7],
            (int)Ipv6[8],(int)Ipv6[9],(int)Ipv6[10],(int)Ipv6[11],
            (int)Ipv6[12],(int)Ipv6[13],(int)Ipv6[14],(int)Ipv6[15],
            (int)Port)};
    }

}
