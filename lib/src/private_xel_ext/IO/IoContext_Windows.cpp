#include <xel_ext/IO/IoContext.hpp>
#include <mutex>
#include <cinttypes>

#ifdef X_SYSTEM_WINDOWS

X_NS {

    static void InitWinsock() {
        WSADATA WsaData;
        if (WSAStartup(MAKEWORD(2,2), &WsaData)) {
            Fatal("WsaData Error");
        }
    }
    static void CleanupWinsock() {
        WSACleanup();
    }
    static xScopeGuard WSAEnv = { InitWinsock, CleanupWinsock };

    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
        _Poller = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
        if (_Poller == INVALID_HANDLE_VALUE) {
            return false;
        }
        X_DEBUG_PRINTF("xIoContext::Init: Instance=%p Poller=%p\n", this, _Poller);
        return true;
    }

    void xIoContext::Clean()
    {
        X_DEBUG_PRINTF("xIoContext::Clean: Instance=%p Poller=%p\n", this, _Poller);

        assert(_DeferredOperationList.IsEmpty());
        assert(_PendingOperationList.IsEmpty());
        CloseHandle(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        DWORD IoSize = 0;
        ULONG_PTR CompleteKeys = 0;
        LPOVERLAPPED OverlappedPtr = NULL;

        OVERLAPPED_ENTRY EventEntries[256];
        ULONG EventCount = 0;
        BOOL Result = GetQueuedCompletionStatusEx (_Poller, EventEntries, (ULONG)Length(EventEntries), &EventCount, (TimeoutMS < 0 ? INFINITE : (DWORD)TimeoutMS), FALSE);
        if (!Result) {
            if (ERROR_ABANDONED_WAIT_0 == GetLastError()) {
                Fatal("xIoContext::LoopOnce, Invalid _Poller");
            }
            return;
        }

        for (ULONG i = 0 ; i < EventCount ; ++i) {
            auto & Event = EventEntries[i];
            auto ReactorPtr = (iBufferedIoReactor*)Event.lpCompletionKey;
            auto OverlappedPtr = Event.lpOverlapped;

            auto EventType = ReactorPtr->GetEventType(Event.lpOverlapped);
            X_DEBUG_PRINTF("xIoContext::LoopOnce, ReactorPtr=%p, lpOverlapped=%p, Transfered=%zi, EventType=%i\n", ReactorPtr, Event.lpOverlapped, (size_t)Event.dwNumberOfBytesTransferred, (int)EventType);

            if (EventType == eIoEventType::Error) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            // process read:
            if (EventType == eIoEventType::InReady) {
                ReactorPtr->SetReadTransfered(Event.dwNumberOfBytesTransferred);
                ReactorPtr->OnIoEventInReady();
            }
            if (!ReactorPtr->IsAvailable()) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            // process write:
            if (EventType == eIoEventType::OutReady) {
                ReactorPtr->SetWriteTransfered(Event.dwNumberOfBytesTransferred);
                ReactorPtr->OnIoEventOutReady();
            }
            if (!ReactorPtr->IsAvailable()) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            // do extra cleanup:

        }

        // exec pending operations:
        _DeferredOperationList.GrabListTail(_PendingOperationList);
        for (auto & Node : _DeferredOperationList) {
            if (!Node.PersistentDeferredOperation) {
                _DeferredOperationList.Remove(Node);
            }

            // TODO: process pending operation:
            auto & IoReactor = (iIoReactor&)Node;
            IoReactor.OnDeferredOperation();
        }

        (void)Result;
    }

}


#endif
