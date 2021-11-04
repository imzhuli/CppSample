#include "./get_mac_if.hpp"

xIfInterfaceList GetIfMac()
{
    char AddressBuffer[std::max(32, std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN))];
    struct ifaddrs *iflist;
    struct ifaddrs *iface;
    if (getifaddrs(&iflist)) {
        return {};
    }
    xIfInterfaceList ResultList;
    for (iface = iflist; iface; iface = iface->ifa_next) {
        xIfInterface Interface;
        Interface.Name = iface->ifa_name;
        if (iface->ifa_addr) {
            if(iface->ifa_addr->sa_family == AF_PACKET) {
                auto *s = (struct sockaddr_ll*)iface->ifa_addr;
                snprintf(AddressBuffer, sizeof(AddressBuffer), "%02X:%02X:%02X:%02X:%02X:%02X",
                         s->sll_addr[0],
                         s->sll_addr[1],
                         s->sll_addr[2],
                         s->sll_addr[3],
                         s->sll_addr[4],
                         s->sll_addr[5]);
                Interface.HardwareAddress = AddressBuffer;
            }
            else if(iface->ifa_addr->sa_family == AF_INET) {
                auto *s = (struct sockaddr_in*)iface->ifa_addr;
                inet_ntop(AF_INET, &s->sin_addr, AddressBuffer, sizeof(AddressBuffer));
                Interface.InetAddress = AddressBuffer;
            }
            else if(iface->ifa_addr->sa_family == AF_INET6) {
                auto *s = (struct sockaddr_in6*)iface->ifa_addr;
                inet_ntop(AF_INET6, &s->sin6_addr, AddressBuffer, sizeof(AddressBuffer));
                Interface.Inet6Address = AddressBuffer;
            }
        }
        if (Interface.HardwareAddress[0] || Interface.InetAddress[0] || Interface.Inet6Address[0]) {
            ResultList.push_back(std::move(Interface));
        }
    }
    freeifaddrs(iflist);
    return ResultList;
}

