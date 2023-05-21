#include <xel_ext/IO/IoContext.hpp>
#include <cinttypes>

#ifdef X_SYSTEM_LINUX

#include <sys/eventfd.h>

X_NS {

    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
#if defined(X_SYSTEM_ANDROID) && X_SYSTEM_ANDROID < 23
        _Poller = epoll_create(2048);
#else
        _Poller = epoll_create1(EPOLL_CLOEXEC);
#endif
        if (-1 == _Poller) {
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
        return true;
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
        struct epoll_event Events[128];
        int Total = epoll_wait(_Poller, Events, (int)Length(Events), TimeoutMS < 0 ? -1 : TimeoutMS);
        for (int i = 0 ; i < Total ; ++i) {
            auto & EV = Events[i];
            auto ReactorPtr = (iIoReactor*)EV.data.ptr;

            if (!ReactorPtr->IsAvailable()) {
                continue;
            }

            if (EV.events & (EPOLLERR | EPOLLHUP)) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            if (EV.events & EPOLLIN) {
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    ReactorPtr->OnIoEventError();
                    continue;
                }
            }

            if (EV.events & EPOLLOUT) {
                ReactorPtr->OnIoEventOutReady();
                if (!ReactorPtr->IsAvailable()) {
                    ReactorPtr->OnIoEventError();
                    continue;
                }
            }
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
            int _UserEventFd = -1;
        };

        bool xUserEventTrigger::Init(xIoContext * IoContextPtr)
        {
            _UserEventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
            if (-1 == _UserEventFd) {
                return false;
            }
            auto Guard = xScopeGuard([&]{ close(_UserEventFd); });

            struct epoll_event Event = {};
            Event.data.ptr = this;
            Event.events = EPOLLET | EPOLLIN;
            if (-1 == epoll_ctl(*IoContextPtr, EPOLL_CTL_ADD, _UserEventFd, &Event)) {
                X_DEBUG_PRINTF("xTcpConnection::Init failed to register epoll event\n");
                return false;
            }

            Guard.Dismiss();
            return true;
        }

        void xUserEventTrigger::Clean()
        {
            close(X_DEBUG_STEAL(_UserEventFd, -1));
        }

        void xUserEventTrigger::OnIoEventInReady()
        {
            uint64_t PseudoData;
            while(true) {
                auto Result = read(_UserEventFd, &PseudoData, 8);
                if (Result < 0) {
                    auto Error = errno;
                    assert(Error == EAGAIN);
                    break;
                }
                assert(Result == 8);
            }
        }

        void xUserEventTrigger::Trigger()
        {
            uint64_t PseudoData = 1;
            auto Result = write(_UserEventFd, &PseudoData, 8);
            if (Result < 0) {
                auto Error = errno;
                assert(Error == EAGAIN);
                return;
            }
            assert(Result == 8);
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
