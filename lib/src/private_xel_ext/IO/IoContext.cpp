#include <xel_ext/IO/IoContext.hpp>

#if   defined(X_SYSTEM_DARWIN)
#include <sys/event.h>
#elif defined(X_SYSTEM_LINUX)
#include <sys/eventfd.h>
#endif

X_NS
{

#ifdef X_SYSTEM_DARWIN
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
            xIoContext * _IoContextPtr = nullptr;
            static const uintptr_t _UserEventIdent = 0;
        };

        bool xUserEventTrigger::Init(xIoContext * IoContextPtr)
        {
            _IoContextPtr = IoContextPtr;
            return true;
        }

        void xUserEventTrigger::Clean()
        {
            X_DEBUG_RESET(_IoContextPtr);
        }

        void xUserEventTrigger::Trigger()
        {
            struct kevent event;
            EV_SET(&event, _UserEventIdent, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
            kevent(*_IoContextPtr, &event, 1, NULL, 0, NULL);
        }

    }
#elif defined(X_SYSTEM_LINUX)
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
                    assert(EAGAIN == errno);
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
                assert(EAGAIN == errno);
                return;
            }
            assert(Result == 8);
        }
    }

#elif defined(X_SYSTEM_WINDOWS)
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

        private:
            void OnIoEventInReady() override;

            xIoContext * _IoContextPtr X_DEBUG_INIT(nullptr);
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

        void xUserEventTrigger::Trigger() {
            PostQueuedCompletionStatus(*_IoContextPtr, 0, (ULONG_PTR)this, nullptr);
        }
    }

#endif

    bool xIoContext::SetupUserEventTrigger()
    {
        auto TriggerPtr = new (std::nothrow) __io_detail__::xUserEventTrigger();
        if (!TriggerPtr) {
            return false;
        }
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