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
        class iUserEventTrigger {
        public:
            virtual void Trigger() = 0;
        };

    public:
        X_API_MEMBER bool Init();
        X_API_MEMBER void Clean();
        X_API_MEMBER void LoopOnce(int TimeoutMS = 50);

        X_INLINE iUserEventTrigger * GetUserEventTrigger() const { return _UserEventTriggerPtr; }
        X_INLINE operator xEventPoller () const { return _Poller; }
        X_INLINE void DeferOperation(xIoReactorNode & Node) {
            _PendingOperationList.GrabTail(Node);
        }

    private:
        X_PRIVATE_MEMBER bool SetupUserEventTrigger();
        X_PRIVATE_MEMBER void CleanUserEventTrigger();

    private:
        xEventPoller          _Poller X_DEBUG_INIT(InvalidEventPoller);
        xIoReactorList        _DeferredOperationList;
        xIoReactorList        _PendingOperationList;
        iUserEventTrigger *   _UserEventTriggerPtr = nullptr;
    };

    #if defined(X_SYSTEM_WINDOWS)
    enum struct eIoEventType
    {
        Error,
        InReady,
        OutReady,
        Closed,
        Ignored,
    };
    #endif

    class iIoReactor
    : public xIoReactorNode
    , private xNonCopyable
    {
    public:
        virtual void OnDeferredOperation() { Pass(); }
        virtual void OnIoEventInReady()    { Pass(); }
        virtual void OnIoEventOutReady()   { Pass(); }
        virtual void OnIoEventError()      { Pass(); }

        X_PRIVATE_INLINE bool IsAvailable() const { return !_StatusFlags; }
        X_PRIVATE_INLINE bool IsDisabled() const  { return _StatusFlags & SF_Disabled; }
        X_PRIVATE_INLINE bool HasError() const    { return _StatusFlags & SF_Error; }

    #if defined(X_SYSTEM_WINDOWS)
        virtual eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) = 0;
    #elif defined(X_SYSTEM_LINUX)
    #elif defined(X_SYSTEM_DARWIN)
    #elif
        #error "Unsupported system"
    #endif

    private:
        static constexpr const uint8_t SF_Disabled = 0x01;
        static constexpr const uint8_t SF_Error    = 0x02;
        uint8_t _StatusFlags = SF_Disabled;

    protected:
        X_INLINE void SetAvailable()   { _StatusFlags = 0; }
        X_INLINE void SetEnabled()     { _StatusFlags &= ~SF_Disabled; }
        X_INLINE void SetDisabled()    { _StatusFlags |= SF_Disabled; }
        X_INLINE void ClearError()     { _StatusFlags &= ~SF_Error; }
        X_INLINE void SetError()       { _StatusFlags |= SF_Error; }

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
        static constexpr const size_t InternalReadBufferSizeForTcp  = 2 * MaxPacketSize;
        static constexpr const size_t InternalReadBufferSizeForUdp = 8192;
        static constexpr const size_t InternalReadBufferSize =
            (InternalReadBufferSizeForTcp > InternalReadBufferSizeForUdp) ?
            InternalReadBufferSizeForTcp : InternalReadBufferSizeForUdp;
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

        sockaddr_storage    _RemoteAddress;
        int                 _RemoteAddressLength;

    #else
        size_t _ReadBufferDataSize;

        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _WriteBufferPtr;
    #endif
    };

}
