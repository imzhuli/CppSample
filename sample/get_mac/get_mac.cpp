
#include <iostream>
#include <sstream>
using namespace std;

#include <sys/types.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#include <string.h>
#include <assert.h>

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
        if (iface->ifa_addr && (iface->ifa_addr)->sa_family == AF_PACKET) {
        struct sockaddr_ll *s = (struct sockaddr_ll*)iface->ifa_addr;
        snprintf(MacBuffer, sizeof(MacBuffer), "%s // %s, (%i): %02X:%02X:%02X:%02X:%02X:%02X",
                 iface->ifa_name,
                 HATypename(s->sll_pkttype),(int)s->sll_pkttype,
                 s->sll_addr[0],
                 s->sll_addr[1],
                 s->sll_addr[2],
                 s->sll_addr[3],
                 s->sll_addr[4],
                 s->sll_addr[5]);
        ss << MacBuffer << ";"  << std::endl;
        }
    }
    auto s = ss.str();
    snprintf(buffer, bufferSize, "%s", s.c_str());
    freeifaddrs(iflist);
}

int main(int argc, char **  argv)
{
    char Buffer[1024];
    PrintMac(Buffer, sizeof(Buffer));
    cout << Buffer << endl;
}