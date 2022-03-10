#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec/Util/IndexedStorage.hpp>
#include <zec/Util/Chrono.hpp>
#include <string>

ZEC_NS
{
    class xIoCaster;

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

    class xIoContext
    : xNonCopyable
    {
    public:
        struct iResumable : xListNode {
        protected:
            virtual void OnResume() {};
        private:
            friend class xIoContext;
        };

    public:
        ZEC_API_MEMBER bool Init();
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER void LoopOnce(int TimeoutMS);

        ZEC_INLINE void Resume(iResumable & Resumable) { _ResumeList.GrabTail(Resumable); }

    private:
        xDummy<16>             _Native;
        xDummy<24>             _WorkGuard;
        xList<iResumable>      _ResumeList;
        friend class xIoCaster;
    };

}
