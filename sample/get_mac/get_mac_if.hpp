#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_link.h>
#include <ifaddrs.h>

struct xIfInterface
{
    std::string Name;
    std::string HardwareAddress;
    std::string InetAddress;
    std::string Inet6Address;
};

using xIfInterfaceList = std::vector<xIfInterface>;

extern xIfInterfaceList GetIfMac();
