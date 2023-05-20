#include <xel_ext/IO/IoContext.hpp>
#include <cinttypes>

#ifdef X_SYSTEM_DARWIN

X_NS {


    bool xIoContext::Init()
    {
        _Poller = kqueue();
        if (_Poller == InvalidEventPoller) {
            return false;
        }
        auto PollerGuard = xScopeGuard([&]{
            close(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
        });

        if (!SetupUserEventTrigger()) {
            return false;
        }
        auto TriggerGuard = xScopeGuard([this]{ CleanUserEventTrigger(); });

        TriggerGuard.Dismiss();
        PollerGuard.Dismiss();
        return false;
    }

    void xIoContext::Clean()
    {
        assert(_DeferredOperationList.IsEmpty());
        assert(_PendingOperationList.IsEmpty());
        CleanUserEventTrigger();
        close(X_DEBUG_STEAL(_Poller, InvalidEventPoller));
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        struct kevent Events[128];
        struct timespec TS = {
            TimeoutMS / 1000,
            ((long)TimeoutMS % 1000) * 1000000,
        };
        int Total = kevent(_Poller, NULL, 0, Events, Length(Events), TimeoutMS < 0 ? nullptr : &TS);
        for (int i = 0 ; i < Total ; ++i) {
            auto & EV = Events[i];
            auto ReactorPtr = (iIoReactor*)EV.udata;

            X_DEBUG_PRINTF("Kevent: id=%" PRIxPTR "\n", EV.ident);
            if (!ReactorPtr->IsAvailable()) {
                continue;
            }

            if (EV.flags & EV_ERROR) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            if (EV.filter == EVFILT_READ) {
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    ReactorPtr->OnIoEventError();
                    continue;
                }
            }

            if (EV.filter == EVFILT_WRITE) {
                ReactorPtr->OnIoEventOutReady();
                if (!ReactorPtr->IsAvailable()) {
                    ReactorPtr->OnIoEventError();
                    continue;
                }
            }

            /* this is not neccessary
            if (EV.filter == EVFILT_USER) {
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    ReactorPtr->OnIoEventError();
                    continue;
                }
            }
            */
        }

        _DeferredOperationList.GrabListTail(_PendingOperationList);
        for (auto & Node : _DeferredOperationList) {
            _DeferredOperationList.Remove(Node);
            auto & IoReactor = (iIoReactor&)Node;
            IoReactor.OnDeferredOperation();
        }

    }

    namespace __io_detail__
    {

        class xUserEventTrigger final
        : public iIoReactor
        , public xIoContext::iUserEventTrigger
        {
        public:
            X_API_MEMBER bool Init(xIoContext * IoContextPtr);
            X_API_MEMBER void Clean();
            X_API_MEMBER void Trigger() override;

        private:
            void OnIoEventInReady() override;

        private:
            static const uintptr_t _UserEventIdent = 0;
        };

        // TODO: implementaion

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
