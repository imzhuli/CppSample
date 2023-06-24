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
        CleanErrorList();
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
                ReactorPtr->SetError();
                ProcessError(*ReactorPtr);
                continue;
            }

            if (EV.events & EPOLLIN) {
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ProcessError(*ReactorPtr);
                    }
                    continue;
                }
            }

            if (EV.events & EPOLLOUT) {
                ReactorPtr->OnIoEventOutReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ProcessError(*ReactorPtr);
                    }
                    continue;
                }
            }
        }
        ProcessErrorList();
    }

}

#endif
