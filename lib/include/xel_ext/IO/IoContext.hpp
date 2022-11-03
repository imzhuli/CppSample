#pragma once
#include "./IoBase.hpp"
#include <xel/Common.hpp>
#include <xel/List.hpp>
#include <xel/Util/IndexedStorage.hpp>
#include <xel/Util/Chrono.hpp>
#include <string>

X_NS
{
    class xIoContext;
    class iIoReactor;
    struct xIoReactorNode : xListNode {};
    using  xIoReactorList = xList<xIoReactorNode>;

    class xIoContext
    : xNonCopyable
    {
    public:
        X_API_MEMBER bool Init();
        X_API_MEMBER void Clean();
        X_API_MEMBER void LoopOnce(int TimeoutMS);

    private:
        xEventPoller     _Poller X_DEBUG_INIT(InvalidEventPoller);
        xIoReactorList   _PendingErrorList;
    };

    class iIoReactor
    {
    public:
        virtual void OnIoEventInReady()  { Fatal("NotImplemented"); }
        virtual void OnIoEventOutReady() { Fatal("NotImplemented"); }
        virtual void OnIoEventError()    { Fatal("NotImplemented"); }
        virtual void OnIoEventReadFinished(void * DataPtr, size_t DataSize)  { Fatal("NotImplemented"); }
        virtual void OnIoEventWriteFinished(void * DataPtr, size_t DataSize) { Fatal("NotImplemented"); }
    };

}
