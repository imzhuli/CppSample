#include "NetBase.hpp"

ZEC_NS
{
    static_assert(std::is_standard_layout_v<xNetAddress>);
    static_assert(sizeof(xNetAddress) == 2 + 16 + 2);
    static_assert(sizeof(xNetAddress) == sizeof(xNetAddress::xKeyType));
    static_assert(sizeof(xNetAddress::xKeyType) == std::tuple_size<xNetAddress::xKeyType>());

    std::string xNetAddress::ToString() const
    {
        char Buffer[64];
        if (Type == eUnknown) {
            return "Unknown";
        }
        if (Type == eIpv4) {
            return {Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d:%d",
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
            "%02x:%02x:%02x:%02x",
            (int)Ipv6[0],(int)Ipv6[1],(int)Ipv6[2],(int)Ipv6[3],
            (int)Ipv6[4],(int)Ipv6[5],(int)Ipv6[6],(int)Ipv6[7],
            (int)Ipv6[8],(int)Ipv6[9],(int)Ipv6[10],(int)Ipv6[11],
            (int)Ipv6[12],(int)Ipv6[13],(int)Ipv6[14],(int)Ipv6[15])};
    }

    xNetAddress xNetAddress::Make(const char * IpStr, uint16_t Port)
    {
        boost::system::error_code ErrorCode;
        auto Address = asio::ip::make_address(IpStr, ErrorCode);
        if (ErrorCode) {
            return {};
        }
        xNetAddress Ret;
        memset(Ret.IpStorage, 0, sizeof(Ret.IpStorage));
        if (Address.is_v4()) {
            Ret.Type = xNetAddress::eIpv4;
            auto AddrBytes = Address.to_v4().to_bytes();
            memcpy(Ret.Ipv4, AddrBytes.data(), sizeof(Ret.Ipv4));
        } else if (Address.is_v6()) {
            Ret.Type = xNetAddress::eIpv6;
            auto AddrBytes = Address.to_v6().to_bytes();
            memcpy(Ret.Ipv6, AddrBytes.data(), sizeof(Ret.Ipv6));
        }
        Ret.Port = Port;
        return Ret;
    }

    xNetAddress xNetAddress::MakeV4(const char * IpStr, uint16_t Port)
    {
        try {
            auto Address = asio::ip::make_address_v4(IpStr);
            auto bytes = Address.to_bytes();

            xNetAddress Ret;
            memset(Ret.IpStorage, 0, sizeof(Ret.IpStorage));
            memcpy(Ret.Ipv4, bytes.data(), sizeof(Ret.Ipv4));
            Ret.Type = eIpv4;
            Ret.Port = Port;
            return Ret;
        }
        catch (...) {}
        return {};
    }

    xNetAddress xNetAddress::MakeV6(const char * IpStr, uint16_t Port)
    {
        try {
            auto Address = asio::ip::make_address_v6(IpStr);
            auto bytes = Address.to_bytes();

            xNetAddress Ret;
            memset(Ret.IpStorage, 0, sizeof(Ret.IpStorage));
            memcpy(Ret.Ipv6, bytes.data(), sizeof(Ret.Ipv6));
            Ret.Type = eIpv6;
            Ret.Port = Port;
            return Ret;
        }
        catch (...) {}
        return {};
    }

}
