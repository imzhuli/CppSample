#include "./get_mac_rtnetlink.hpp"
#include <iostream>
using namespace std;

// netlink package is base on packets, 
// but there might be large response consist of small packets.
#define SZ 81920 

xNetlinkInterfaceList GetNetlinkMac()
{

    // Send
    typedef struct
    {
        struct nlmsghdr nh;
        struct ifinfomsg ifi;
    } Req_getlink;
    assert(NLMSG_LENGTH(sizeof(struct ifinfomsg)) == sizeof(Req_getlink));
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd == -1) {
        return {};
    }

    auto request = Req_getlink{
        .nh = {
            .nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
            .nlmsg_type = RTM_GETLINK,
            .nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT,
            .nlmsg_seq = 0,
            .nlmsg_pid = 0},
        .ifi = {
            .ifi_family = AF_UNSPEC,
            .ifi_type = 0,
            .ifi_index = 0,
            .ifi_flags = 0,
            .ifi_change = 0,
        }};
    auto sendbytes = send(fd, &request, sizeof(request), 0);
    if (sendbytes != sizeof(request)) {
        // LOGE("Failed to send netlink request, socket=%i, sendbytes=%i, errno=%i", (int)fd, (int)sendbytes, errno);
        close(fd);
        return {};
    }

    // Receive
    char recvbuf[SZ] = {};
    int len = 0;
    for (char *p = recvbuf;;)
    {
        const int seglen = recv(fd, p, sizeof(recvbuf) - len, 0);

        if (seglen <= 0) {
            auto err = errno;
            if (err == EAGAIN) {
                cerr << "netlinke response buffer is too small, current size is " << SZ << endl;                
            }
            close(fd);
            return {};
        };

        len += seglen;
        auto MessageType = ((struct nlmsghdr *)p)->nlmsg_type;
        if (MessageType == NLMSG_ERROR) {
            cerr << "netlink returns error" << endl;
            close(fd);
            return {};
        }
        if (MessageType == NLMSG_DONE) {
            break;
        }
        p += seglen;
    }

    xNetlinkInterfaceList ResultList;
    struct nlmsghdr *nh = (struct nlmsghdr *)recvbuf;
    for (; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len))
    {
        if (nh->nlmsg_type == NLMSG_DONE) {
            break;
        }

        xNetlinkInterface Interface;

        struct ifinfomsg *ifm = (struct ifinfomsg *)NLMSG_DATA(nh);    
        Interface.Index = ifm->ifi_index;
    
        struct rtattr *rta = IFLA_RTA(ifm); 
        int rtl = RTM_PAYLOAD(nh);
        for (; RTA_OK(rta, rtl); rta = RTA_NEXT(rta, rtl))
        {
            switch (rta->rta_type) {
            case IFLA_IFNAME: {
                Interface.Name =  (const char *)RTA_DATA(rta);
                break;
            }
            case IFLA_ADDRESS: {
                char HwAddr[18];
                const unsigned char * Data = (const unsigned char*)RTA_DATA(rta);
                snprintf(HwAddr, sizeof(HwAddr), "%02X:%02X:%02X:%02X:%02X:%02X", 
                    (unsigned int)Data[0], 
                    (unsigned int)Data[1], 
                    (unsigned int)Data[2], 
                    (unsigned int)Data[3], 
                    (unsigned int)Data[4], 
                    (unsigned int)Data[5]);
                Interface.HardwareAddress = HwAddr;   
                break;             
            }
            default:
                break;
            }            
        }
        ResultList.push_back(std::move(Interface));
    }
    close(fd);
    return ResultList;
}
