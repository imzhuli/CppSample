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

        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        alignas(max_align_t) ubyte _Dummy[32];

    private:
        friend class __detail__::IOUtil;
    };

}
