#include <zec_ext/IO/IoContext.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    std::string xNetAddress::ToString() const
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

        return {Buffer, (size_t)sprintf(Buffer,
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x:"
            "%02x:%02x:%02x:%02x",

            (int)Ipv6[0],
            (int)Ipv6[1],
            (int)Ipv6[2],
            (int)Ipv6[3],

            (int)Ipv6[4],
            (int)Ipv6[5],
            (int)Ipv6[6],
            (int)Ipv6[7],

            (int)Ipv6[8],
            (int)Ipv6[9],
            (int)Ipv6[10],
            (int)Ipv6[11],

            (int)Ipv6[12],
            (int)Ipv6[13],
            (int)Ipv6[14],
            (int)Ipv6[15])};
    }

    bool xIoContext::Init()
    {
        auto & IoContextHolder = NativeIoContextHolderRef(Native());
        IoContextHolder.Create();
        return IoContextHolder.IsValid();
    }

    void xIoContext::Clean()
    {
        auto & IoContextHolder = NativeIoContextHolderRef(Native());
        IoContextHolder.Destroy();
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        auto & IoContextHolder = NativeIoContextHolderRef(Native());
        IoContextHolder->run_for(std::chrono::microseconds(TimeoutMS));
    }

}
