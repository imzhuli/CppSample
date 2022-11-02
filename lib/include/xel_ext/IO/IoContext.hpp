#pragma once
#include "./IoBase.hpp"
#include <xel/Common.hpp>
#include <xel/List.hpp>
#include <xel/Util/IndexedStorage.hpp>
#include <xel/Util/Chrono.hpp>
#include <string>

X_NS
{
    class xIoHandle
    {
    public:
        X_INLINE xIoHandle(void * NativeObjectPtr) : ObjectPtr(NativeObjectPtr) {}
        template<typename T>
        X_INLINE T* AsPtr() const { return static_cast<T*>(ObjectPtr); };
        template<typename T>
        X_INLINE T& AsRef() const { return *static_cast<T*>(ObjectPtr); };
    private:
        void * ObjectPtr;
    };

    class xIoContext
    : xNonCopyable
    {
    public:
        X_API_MEMBER bool Init();
        X_API_MEMBER void Clean();
        X_API_MEMBER void LoopOnce(int TimeoutMS);

    private:
    };

}
