#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec/Util/IniReader.hpp>
#include <zec/Util/Chrono.hpp>
#include <string>

ZEC_NS
{

    class xIoHandle
    {
    public:
        ZEC_INLINE xIoHandle(void * NativeObjectPtr) : ObjectPtr(NativeObjectPtr) {}
        template<typename T>
        ZEC_INLINE T* AsPtr() const { return static_cast<T*>(ObjectPtr); };
        template<typename T>
        ZEC_INLINE T& AsRef() const { return *static_cast<T*>(ObjectPtr); };
    private:
        void * ObjectPtr;
    };

    class xIoService
    : xNonCopyable
    {
    public:
        ZEC_API_MEMBER bool Init(const xIniReader & ConfigReader);
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER void LoopOnce(int TimeoutMS);

    private:
        xDummy<16>                _Native;
        uint64_t                  _ExpireTimeout;
        xList<xExpiringNode>      _ExpiringNodelist;
    };



}
