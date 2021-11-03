#include "./get_mac_if.hpp"

[[maybe_unused]]
static const char * HATypename(short type)
{
    switch(type) {
        case PACKET_HOST:
            return "PACKET_HOST";
        case PACKET_BROADCAST:
            return "PACKET_BROADCAST";
        case PACKET_MULTICAST:
            return "PACKET_MULTICAST";
        case PACKET_OTHERHOST:
            return "PACKET_OTHERHOST";
        case PACKET_OUTGOING:
            return "PACKET_OUTGOING";
        case PACKET_LOOPBACK:
            return "PACKET_LOOPBACK (invisible to userspace)";
        case PACKET_USER:
            return "PACKET_USER (invisible to userspace)";
    }

    return "Unkown";
}

[[maybe_unused]]
static size_t GetSockSize(const sockaddr* AddrPtr) {
    if (AddrPtr->sa_family == AF_INET) {
        return sizeof(sockaddr_in);
    } 
    if (AddrPtr->sa_family == AF_INET6) {
        return sizeof(sockaddr_in6);
    }
    return 0;
}

xIfInterfaceList GetIfMac()
{
    char MacBuffer[18];

    struct ifaddrs *iflist;
    struct ifaddrs *iface;
    if (getifaddrs(&iflist)) {        
        return {};
    }

    xIfInterfaceList ResultList;
    for (iface = iflist; iface; iface = iface->ifa_next) {
        xIfInterface Interface; 
        Interface.Name = iface->ifa_name;
        // cout << "name:" << iface->ifa_name << ", flags:" << std::hex << iface->ifa_flags << std::dec << "; ";

        if (iface->ifa_addr) {
            if(iface->ifa_addr->sa_family == AF_PACKET) {
                struct sockaddr_ll *s = (struct sockaddr_ll*)iface->ifa_addr;
                snprintf(MacBuffer, sizeof(MacBuffer), "%02X:%02X:%02X:%02X:%02X:%02X",
                         s->sll_addr[0],
                         s->sll_addr[1],
                         s->sll_addr[2],
                         s->sll_addr[3],
                         s->sll_addr[4],
                         s->sll_addr[5]);
                Interface.HardwareAddress = MacBuffer;
                // cout << "AF_PACKET:" << MacBuffer << "; ";
            }
            else if (iface->ifa_addr->sa_family == AF_INET){
                // cout << "INET" << " " ;
            }
            else if (iface->ifa_addr->sa_family == AF_INET6){
                // cout << "INET_6" << " " ;
            }
            else {
                // cout << "sa_family: " << iface->ifa_addr->sa_family << " " ;
            }
        } else {
            // cout << "ifa_addr_nil" << " ";
        }

        // if (iface->ifa_data) {
        //     struct rtnl_link_stats *stats = (struct rtnl_link_stats *)iface->ifa_data;
        //     char StateBuffer[1024];
        //     snprintf(StateBuffer, sizeof(StateBuffer), "\t\ttx_packets = %10u; rx_packets = %10u\n"
        //             "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
        //             stats->tx_packets, stats->rx_packets,
        //             stats->tx_bytes, stats->rx_bytes);
        // } else {
        //     cout << "ifa_data_nil" << " ";
        // }

        // if (iface->ifa_netmask) {
        //     cout << "netamsk:" << zec::StrToHex(iface->ifa_netmask, GetSockSize(iface->ifa_netmask)) << " ";
        // } else {
        //     cout << "netmask_nil" << " ";
        // }
        if (Interface.HardwareAddress[0]) {
            ResultList.push_back(std::move(Interface));
        }
    }    
    freeifaddrs(iflist);
    return ResultList;
}
