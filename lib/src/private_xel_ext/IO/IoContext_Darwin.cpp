#include <xel_ext/IO/IoContext.hpp>
#include <cinttypes>
#include <sys/types.h>

#ifdef X_SYSTEM_DARWIN

#include <sys/event.h>

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
        struct kevent Events[128];
        struct timespec TS = {
            TimeoutMS / 1000,
            ((long)TimeoutMS % 1000) * 1000000,
        };
        int Total = kevent(_Poller, NULL, 0, Events, Length(Events), TimeoutMS < 0 ? nullptr : &TS);
        for (int i = 0 ; i < Total ; ++i) {
            auto & EV = Events[i];
            auto ReactorPtr = (iIoReactor*)EV.udata;

            // X_DEBUG_PRINTF("Kevent: id=%" PRIxPTR "\n", EV.ident);
            if (!ReactorPtr->IsAvailable()) {
                continue;
            }

            if (EV.flags & EV_ERROR) {
                ReactorPtr->SetError();
                ProcessError(*ReactorPtr);
                continue;
            }

            if (EV.filter == EVFILT_READ) {
                ReactorPtr->OnIoEventInReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ProcessError(*ReactorPtr);
                    }
                    continue;
                }
            }

            if (EV.filter == EVFILT_WRITE) {
                ReactorPtr->OnIoEventOutReady();
                if (!ReactorPtr->IsAvailable()) {
                    if (ReactorPtr->HasError()) {
                        ProcessError(*ReactorPtr);
                    }
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
        ProcessErrorList();
    }

}


#endif
