#include <xel_ext/IO/IoContext.hpp>

#ifdef X_SYSTEM_DARWIN

X_NS {


    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
        _Poller = kqueue();
        return false;
    }

    void xIoContext::Clean()
    {
        close(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        Fatal("Not implemented");
    }

}


#endif
