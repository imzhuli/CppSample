#include <xel_ext/IO/IoContext.hpp>

#ifdef X_SYSTEM_WINDOWS

X_NS {

    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
        _Poller = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
        if (_Poller == INVALID_HANDLE_VALUE) {
            return false;
        }
        return true;
    }

    void xIoContext::Clean()
    {
        assert(_PendingErrorList.IsEmpty());
        CloseHandle(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        DWORD IoEventCount = 0;
        ULONG_PTR CompleteKeys = 0;
        LPOVERLAPPED OverlappedPtr = NULL;
        BOOL Result = GetQueuedCompletionStatus(_Poller, &IoEventCount, &CompleteKeys, &OverlappedPtr, (TimeoutMS < 0 ? INFINITE : (DWORD)TimeoutMS));
        if (!Result) {
            return;
        }

        // process error:

        // process read:

        // process write:

        // do extra cleanup:
    }
    
}


#endif
