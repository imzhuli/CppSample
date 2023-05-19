#pragma once
#include "./_.hpp"
#include "./PacketBuffer.hpp"
#include <xel/Common.hpp>
#include <xel/List.hpp>
#include <xel/Util/IndexedStorage.hpp>
#include <xel/Util/Chrono.hpp>
#include <string>

X_NS
{
    class xIoContext;
    class iIoReactor;
    struct xIoReactorNode : xListNode {};
    using  xIoReactorList = xList<xIoReactorNode>;

    class xIoContext
    : xNonCopyable
    {
    public:
        X_API_MEMBER bool Init();
        X_API_MEMBER void Clean();
        X_API_MEMBER void LoopOnce(int TimeoutMS = 50);

        X_INLINE operator xEventPoller () const { return _Poller; }
        X_INLINE void DeferOperation(xIoReactorNode & Node) {
            _PendingOperationList.GrabTail(Node);
        }

    private:
        xEventPoller     _Poller X_DEBUG_INIT(InvalidEventPoller);
        xIoReactorList   _DeferredOperationList;
        xIoReactorList   _PendingOperationList;
    };

    enum struct eIoEventType
    {
        Error,
        InReady,
        OutReady,
        Closed,
    };

    class iIoReactor
    : public xIoReactorNode
    , private xNonCopyable
    {
    public:
        virtual void OnDeferredOperation() { Pass(); }
        virtual void OnIoEventInReady()    { Pass(); }
        virtual void OnIoEventOutReady()   { Pass(); }
        virtual void OnIoEventError()      { Pass(); }

        X_PRIVATE_INLINE bool IsAvailable() const { return _Available; }
    #if defined(X_SYSTEM_WINDOWS)
        virtual eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) = 0;
    #elif defined(X_SYSTEM_LINUX)
    #elif defined(X_SYSTEM_DARWIN)
    #elif
        #error "Unsupported system"
    #endif

    protected:
        X_INLINE void SetUnavailable() { _Available = false; }

    private:
        bool _Available = true;
    };

    class xUserEventTrigger final
    : public iIoReactor
    {
    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER void Trigger();

    private:
        void OnIoEventInReady() override;

    private:
    #if   defined(X_SYSTEM_LINUX)
        int _UserEventFd = -1;
    #elif defined(X_SYSTEM_WINDOWS)
    #elif defined(X_SYSTEM_DARWIN)
        static const uintptr_t _UserEventIdent = 0;
    #endif
    };

    class iBufferedIoReactor
    : public iIoReactor
    {
    public:
    #if defined(X_SYSTEM_WINDOWS)
        X_INLINE void SetReadTransfered(DWORD Size) { _ReadDataSize = Size; }
        X_INLINE void SetWriteTransfered(DWORD Size) { _WriteDataSize = Size; }
    #endif

    protected:
        static constexpr const size_t InternalReadBufferSize  = 8192;
        ubyte  _ReadBuffer[InternalReadBufferSize];

    protected:
    #if defined(X_SYSTEM_WINDOWS)
        DWORD      _ReadFlags;
        DWORD      _ReadDataSize;
        WSABUF     _ReadBufferUsage;
        OVERLAPPED _ReadOverlappedObject;

        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _WriteBufferPtr;
        DWORD               _WriteDataSize;
        WSABUF              _WriteBufferUsage;
        OVERLAPPED          _WriteOverlappedObject;
    #else
        size_t _ReadBufferDataSize;

        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _WriteBufferPtr;
    #endif
    };

}
