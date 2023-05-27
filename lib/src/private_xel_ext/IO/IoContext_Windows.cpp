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
        auto PollerGuard = xScopeGuard([&]{
            CloseHandle(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
        });

        if (!SetupUserEventTrigger()) {
            return false;
        }
        auto TriggerGuard = xScopeGuard([this]{ CleanUserEventTrigger(); });

        TriggerGuard.Dismiss();
        PollerGuard.Dismiss();
        return true;
    }

    void xIoContext::Clean()
    {
        CleanUserEventTrigger();
        CloseHandle(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
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

            auto EventType = ReactorPtr->GetEventType(Event.lpOverlapped);
            X_DEBUG_PRINTF("xIoContext::LoopOnce, ReactorPtr=%p, lpOverlapped=%p, Transfered=%zi, EventType=%i\n", ReactorPtr, Event.lpOverlapped, (size_t)Event.dwNumberOfBytesTransferred, (int)EventType);

            if (EventType == eIoEventType::Error) {
                ReactorPtr->SetError();
                ReactorPtr->OnIoEventError();
                continue;
            }

            // process read:
            if (EventType == eIoEventType::InReady) {
                ReactorPtr->SetReadTransfered(Event.dwNumberOfBytesTransferred);
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ReactorPtr->OnIoEventError();
                    }
                    continue;
                }
            }

            // process write:
            if (EventType == eIoEventType::OutReady) {
                ReactorPtr->SetWriteTransfered(Event.dwNumberOfBytesTransferred);
                ReactorPtr->OnIoEventOutReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ReactorPtr->OnIoEventError();
                    }
                    continue;
                }
            }

            // if (EventType == eIoEventType::Ignored) {
            //     continue;
            // }
        }

    }

    namespace __io_detail__
    {

        class xUserEventTrigger final
        : public iBufferedIoReactor
        , public xIoContext::iUserEventTrigger
        {
        public:
            X_API_MEMBER bool Init(xIoContext * IoContextPtr);
            X_API_MEMBER void Clean();
            X_API_MEMBER void Trigger() override;
            X_API_MEMBER eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) override { return eIoEventType::Ignored; }

        private:
            void OnIoEventInReady() override;

            xIoContext * _IoContextPtr;
        };

        bool xUserEventTrigger::Init(xIoContext * IoContextPtr)
        {
            _IoContextPtr = IoContextPtr;
            return true;
        }

        void xUserEventTrigger::Clean()
        {}

        void xUserEventTrigger::OnIoEventInReady()
        {}

        void xUserEventTrigger::Trigger()
        {
            PostQueuedCompletionStatus(*_IoContextPtr, 0, (ULONG_PTR)this, nullptr);
        }

    }

    bool xIoContext::SetupUserEventTrigger()
    {
        auto TriggerPtr = new __io_detail__::xUserEventTrigger();
        if (!TriggerPtr->Init(this)) {
            return false;
        }
        _UserEventTriggerPtr = TriggerPtr;
        return true;
    }

    void xIoContext::CleanUserEventTrigger()
    {
        auto TriggerPtr = (__io_detail__::xUserEventTrigger *)Steal(_UserEventTriggerPtr);
        TriggerPtr->Clean();
        delete TriggerPtr;
    }

}


#endif
