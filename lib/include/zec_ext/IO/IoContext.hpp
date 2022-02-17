#pragma once
#include <zec/Common.hpp>

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

    private:
        friend class __detail__::IOUtil;
    };

    enum struct eNetAddressType : uint8_t {
        None, Ipv4, Ipv6
    };

    struct xNetAddress {
        eNetAddressType   Type;
        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
        };
    };

}
