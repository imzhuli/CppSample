#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <unistd.h>
#include <asm/types.h>

struct xNetlinkInterface
{
    uint32_t Index;
    std::string Name;
    std::string HardwareAddress;
};

using xNetlinkInterfaceList = std::vector<xNetlinkInterface>;

extern xNetlinkInterfaceList GetNetlinkMac();
