#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xIoContext
    : xNonCopyable
    {
    public:
        ZEC_API_MEMBER bool Init();
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER void LoopOnce(int TimeoutMS);

        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        alignas(max_align_t) ubyte _Dummy[32];

        friend class __detail__::IOUtil;
    };

    struct xNetAddress {
        enum : uint8_t {
            eUnknown, eIpv4, eIpv6
        } Type = eUnknown;

        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
        };

        ZEC_API_MEMBER std::string ToString() const;
    };

}
