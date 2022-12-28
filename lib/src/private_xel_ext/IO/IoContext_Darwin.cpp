#include <xel_ext/IO/IoContext.hpp>
#include <cinttypes>

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
            }
            if (!ReactorPtr->IsAvailable()) {
                ReactorPtr->OnIoEventError();
                continue;
            }

            if (EV.filter == EVFILT_WRITE) {
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
