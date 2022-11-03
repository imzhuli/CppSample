#include <xel_ext/IO/IoContext.hpp>

#ifdef X_SYSTEM_LINUX

X_NS {

    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
        _Poller = epoll_create1(FD_CLOEXEC);
        if (-1 == Poller) {
            return false;
        }
        return true;
    }

    void xIoContext::Clean()
    {
        assert(_PendingErrorList.IsEmpty());
        close(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }
    
}


#endif
