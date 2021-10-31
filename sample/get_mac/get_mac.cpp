#include <zec/String.hpp>

#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <linux/if_packet.h>
#include <ifaddrs.h>
#include <string.h>
#include <assert.h>

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

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

void PrintMac(char * buffer, size_t bufferSize)
{
    char MacBuffer[128];
    snprintf(buffer, bufferSize, "%s", "Hello world!");

    assert(buffer && bufferSize);
    struct ifaddrs *iflist;
    struct ifaddrs *iface;

    if (getifaddrs(&iflist)) {
        buffer[0] = '\0';
        return;
    }

    std::stringstream ss;
    for (iface = iflist; iface; iface = iface->ifa_next) {
        ss << "name:" << iface->ifa_name << ", flags:" << std::hex << iface->ifa_flags << std::dec << "; ";

        if (iface->ifa_addr) {
            if(iface->ifa_addr->sa_family == AF_PACKET) {
                struct sockaddr_ll *s = (struct sockaddr_ll*)iface->ifa_addr;
                snprintf(MacBuffer, sizeof(MacBuffer), "%s(%i:%i:%i): %02X:%02X:%02X:%02X:%02X:%02X",
                         iface->ifa_name,
                         (int)s->sll_protocol, (int)s->sll_hatype, (int)s->sll_halen,
                         s->sll_addr[0],
                         s->sll_addr[1],
                         s->sll_addr[2],
                         s->sll_addr[3],
                         s->sll_addr[4],
                         s->sll_addr[5]);
                ss << "AF_PACKET:" << MacBuffer << "; ";
            }
            else if (iface->ifa_addr->sa_family == AF_INET){
                ss << "INET" << " " ;
            }
            else if (iface->ifa_addr->sa_family == AF_INET6){
                ss << "INET_6" << " " ;
            }
            else {
                ss << "sa_family: " << iface->ifa_addr->sa_family << " " ;
            }
        } else {
            ss << "ifa_addr_nil" << " ";
        }

        if (iface->ifa_data) {
            ss << "ifa_data_ok" << " ";
        } else {
            ss << "ifa_data_nil" << " ";
        }

        if (iface->ifa_netmask) {
            ss << "netamsk:" << zec::StrToHex(iface->ifa_netmask, GetSockSize(iface->ifa_netmask)) << " ";
        } else {
            ss << "netmask_nil" << " ";
        }

        ss << std::endl;
    }
    auto s = ss.str();
    snprintf(buffer, bufferSize, "%s", s.c_str());
    freeifaddrs(iflist);
}


int main(int argc, char **  argv)
{
    char Buffer[4096];
    PrintMac(Buffer, sizeof(Buffer));
    cout << Buffer << endl;
}