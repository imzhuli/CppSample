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
        return true;
    }

    void xIoContext::Clean()
    {
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
            }
            if (!ReactorPtr->IsAvailable()) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            if (EV.events & EPOLLOUT) {
                ReactorPtr->OnIoEventOutReady();
            }
            if (!ReactorPtr->IsAvailable()) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            // extra events:

        }
    }


    bool xUserEventTrigger::Init(xIoContext * IoContextPtr)
    {
        _UserEventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (-1 == _UserEventFd) {
            return false;
        }
        auto Guard = xScopeGuard([&]{ close(_UserEventFd); });

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

#endif
