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

    private:
        X_PRIVATE_MEMBER bool SetupUserEventTrigger();
        X_PRIVATE_MEMBER void CleanUserEventTrigger();

    private:
        xEventPoller          _Poller X_DEBUG_INIT(InvalidEventPoller);
        iUserEventTrigger *   _UserEventTriggerPtr = nullptr;

    #ifdef X_SYSTEM_WINDOWS
    public:
        struct xDeferredEventNode : xListNode, xNonCopyable {};
        using  xDeferredEventList = xList<xDeferredEventNode>;
        void DeferEvent(xDeferredEventNode & Node) { PendingEventList.GrabTail(Node); }
    private:
        xDeferredEventList PendingEventList; // to prevent infinate loop
        xDeferredEventList DeferredEventList;
    #endif
    };

    #if defined(X_SYSTEM_WINDOWS)
    enum struct eIoEventType
    {
        Unspecified,
        Error,
        InReady,
        OutReady,
        Closed,
        Ignored,
        Cleanup, // the event owner is already invalidated, cleanup the overlapped object.
    };
    #endif

    /** NOTE: !!! important !!!
        Due to CompletionPort design, it's very important to process overlapped object AFTER its owner might have been released.
        To unrelate overlapped object and it's owner, I am defining a small object in iBufferedIoReactor to help with that.
        On windows, Io object SHOULD NEVER directly derive from iIoReactor, but iBufferedIoReactor.
    */
    class iIoReactor
    : private xNonCopyable
    {
        friend class xIoContext;
    public:
        virtual void OnIoEventInReady()    { Pass(); }
        virtual void OnIoEventOutReady()   { Pass(); }
        virtual void OnIoEventError()      { Pass(); }

        X_PRIVATE_INLINE bool IsAvailable() const { return !_StatusFlags; }
        X_PRIVATE_INLINE bool IsDisabled() const  { return _StatusFlags & SF_Disabled; }
        X_PRIVATE_INLINE bool HasError() const    { return _StatusFlags & SF_Error; }

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
#ifdef X_SYSTEM_WINDOWS
    , xIoContext::xDeferredEventNode
#endif
    {
    protected:
        static constexpr const size_t InternalReadBufferSizeForTcp  = 2 * MaxPacketSize;
        static constexpr const size_t InternalReadBufferSizeForUdp = 8192;
        static constexpr const size_t InternalReadBufferSize =
            (InternalReadBufferSizeForTcp > InternalReadBufferSizeForUdp) ?
            InternalReadBufferSizeForTcp : InternalReadBufferSizeForUdp;

    #ifndef X_SYSTEM_WINDOWS
        ubyte  _ReadBuffer[InternalReadBufferSize];
    protected:
        size_t              _ReadBufferDataSize;
        xPacketBufferChain  _WriteBufferChain;
        xPacketBuffer *     _WriteBufferPtr;

    #else

    protected:
        friend class xIoContext; // called on completion event port
        X_INLINE void SetReadTransfered(DWORD Size)  { IoBufferPtr->ReadObject.DataSize  = Size; }
        X_INLINE void SetWriteTransfered(DWORD Size) { IoBufferPtr->WriteObject.DataSize = Size; }

    protected:
        struct xOverlappedIoBuffer;
        struct xOverlappedBlock
        {
            xOverlappedIoBuffer *  Outter;
            DWORD                  DataSize;
            OVERLAPPED             OverlappedObject;
        };

        struct xOverlappedIoBuffer
        {
            bool                  DeleteMark;
            xOverlappedBlock      ReadObject;
            ubyte                 ReadBuffer[InternalReadBufferSize];
            xOverlappedBlock      WriteObject;
            xPacketBufferChain    WriteBufferChain;
        };

        xOverlappedIoBuffer * IoBufferPtr;

    #endif
    };

}
