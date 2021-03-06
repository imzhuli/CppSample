#include "./NetBase.hpp"
#include <zec/String.hpp>

ZEC_NS
{
    static_assert(std::is_standard_layout_v<xNetAddress>);
    static_assert(sizeof(xNetAddress) == 2 + 16 + 2);
    static_assert(sizeof(xNetAddress) == sizeof(xNetAddress::xKeyType));
    static_assert(sizeof(xNetAddress::xKeyType) == std::tuple_size<xNetAddress::xKeyType>());

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

    xNetAddress xNetAddress::MakeV4Raw(const void * AddrRaw, uint16_t Port)
    {
        xNetAddress Ret;
        memset(Ret.IpStorage, 0, sizeof(Ret.IpStorage));
        memcpy(Ret.Ipv4, AddrRaw, sizeof(Ret.Ipv4));
        Ret.Type = eIpv4;
        Ret.Port = Port;
        return Ret;
    }

    xNetAddress xNetAddress::MakeV4(const char * IpStr, uint16_t Port)
    {
        try {
            auto Address = asio::ip::make_address_v4(IpStr);
            auto bytes = Address.to_bytes();
            return MakeV4Raw(bytes.data(), Port);
        }
        catch (...) {}
        return {};
    }

    xNetAddress xNetAddress::MakeV6Raw(const void * AddrRaw, uint16_t Port)
    {
        xNetAddress Ret;
        memset(Ret.IpStorage, 0, sizeof(Ret.IpStorage));
        memcpy(Ret.Ipv6, AddrRaw, sizeof(Ret.Ipv6));
        Ret.Type = eIpv6;
        Ret.Port = Port;
        return Ret;
    }

    xNetAddress xNetAddress::MakeV6(const char * IpStr, uint16_t Port)
    {
        try {
            auto Address = asio::ip::make_address_v6(IpStr);
            auto bytes = Address.to_bytes();
            return MakeV6Raw(bytes.data(), Port);
        }
        catch (...) {}
        return {};
    }

    xNetAddress xNetAddress::Parse(const std::string & AddressStr)
    {
        auto TrimAddressStr = Trim(AddressStr);
        auto SpIndex = TrimAddressStr.find(':');
        auto Hostname = TrimAddressStr.substr(0, SpIndex);
        auto Port = static_cast<uint16_t>((SpIndex == TrimAddressStr.npos) ? 0 : atoll(TrimAddressStr.data() + SpIndex + 1));
        return xNetAddress::Make(Hostname.c_str(), Port);
    }

}
