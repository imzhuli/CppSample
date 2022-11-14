#include <xel_ext/IO/IoContext.hpp>
#include <cinttypes>

#ifdef X_SYSTEM_LINUX

X_NS {

    bool xIoContext::Init()
    {
        assert(_Poller == InvalidEventPoller);
        _Poller = epoll_create1(EPOLL_CLOEXEC);
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
}


#endif
