#pragma once
#include "./IoBase.hpp"
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
    struct xIoReactorNode : xListNode { bool PersistentDeferredOperation = false; };
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
        virtual void OnIoEventInReady()  { Pass(); }
        virtual void OnIoEventOutReady() { Pass(); }
        virtual void OnIoEventError()    { Pass(); }

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

    class iBufferedIoReactor
    : public iIoReactor
    {
    protected:
        static constexpr const size_t InternalReadBufferSize  = 8192;
        static constexpr const size_t InternalWriteBufferSize = 8192;

        ubyte  _ReadBuffer[InternalReadBufferSize];
        ubyte  _WriteBuffer[InternalWriteBufferSize];

    #if defined(X_SYSTEM_WINDOWS)
        DWORD      _ReadFlags;
        DWORD      _ReadDataSize;
        WSABUF     _ReadBufferUsage;
        OVERLAPPED _ReadOverlappedObject;

        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _SendingBufferPtr;
        DWORD               _SentDataSize;
        WSABUF              _WriteBufferUsage;
        OVERLAPPED          _WriteOverlappedObject;
    #else
        size_t _ReadBufferDataSize;
        size_t _WriteBufferDataSize;

        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _SendingBufferPtr;
    #endif
    };

}
