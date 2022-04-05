#include "./Common.hpp"

xNetAddress ParseIpAddress(const std::string & AddressStr)
{
    auto TrimAddressStr = zec::Trim(AddressStr);
    auto SpIndex = TrimAddressStr.find(':');
	if (SpIndex == TrimAddressStr.npos) {
		return {};
	}

    auto Hostname = TrimAddressStr.substr(0, SpIndex);
	auto Port = atoll(TrimAddressStr.data() + SpIndex + 1);

    xNetAddress Addr = xNetAddress::Make(Hostname.c_str(), Port);
    if (!Addr) {
        return {};
    }
    return Addr;
}
