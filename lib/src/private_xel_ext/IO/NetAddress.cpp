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

}
