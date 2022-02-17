#pragma once
#include <zec/Common.hpp>

ZEC_NS
{

    class xIoContext
    : xNonCopyable
    {
    public:
        ZEC_API_MEMBER bool Init();
        ZEC_API_MEMBER void Clean();

        ZEC_INLINE void * Native() const { return _RealIoContextPtr; }

    private:
        void * _RealIoContextPtr = nullptr;
    };

}