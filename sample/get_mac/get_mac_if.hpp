#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <linux/if_packet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

struct xIfInterface
{
    std::string Name;
    std::string HardwareAddress;
};

using xIfInterfaceList = std::vector<xIfInterface>;

extern xIfInterfaceList GetIfMac();
