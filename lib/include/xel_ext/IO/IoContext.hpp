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

    /** NOTE: !!! important !!!
        Due to CompletionPort design, it's very important to process overlapped object AFTER its owner might have been released.
        To unrelate overlapped object and it's owner, I am defining a small object in iBufferedIoReactor to help with that.
        On windows, Io object SHOULD NEVER directly derive from iIoReactor, but iBufferedIoReactor.
    */
    class iIoReactor
    : public xListNode
    , private xNonCopyable
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

        X_INLINE void PostError(iIoReactor & IoReactor) {
            if (IoReactor.HasError()) {
                return;
            }
            IoReactor.SetError();
            _ErrorIoReactorList.GrabTail(IoReactor);
        }

    private:
        X_PRIVATE_MEMBER bool SetupUserEventTrigger();
        X_PRIVATE_MEMBER void CleanUserEventTrigger();
        X_INLINE void CleanErrorList() {
            for (auto & IoReactor : _ErrorIoReactorList) {
                xListNode::Unlink(IoReactor);
            }
        }
        X_INLINE void ProcessError(iIoReactor& IoReactor) {
            xListNode::Unlink(IoReactor);
            IoReactor.OnIoEventError();
        }
        X_INLINE void ProcessErrorList() {
            for (auto & IoReactor : _ErrorIoReactorList) {
                ProcessError(IoReactor);
            }
        }

    private:
        xEventPoller          _Poller X_DEBUG_INIT(InvalidEventPoller);
        iUserEventTrigger *   _UserEventTriggerPtr = nullptr;
        xList<iIoReactor>     _ErrorIoReactorList;
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

    class iBufferedIoReactor
    : public iIoReactor
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
        X_INLINE void SetReadTransfered(DWORD Size)  { _IoBufferPtr->ReadObject.DataSize  = Size; }
        X_INLINE void SetWriteTransfered(DWORD Size) { _IoBufferPtr->WriteObject.DataSize = Size; }

    protected:
        struct xOverlappedIoBuffer;
        struct xOverlappedObject
        : private ::xel::xNonCopyable
        {
            xOverlappedIoBuffer *  Outter;
            bool                   AsyncOpMark;
            DWORD                  DataSize;
            WSABUF                 BufferUsage;
            OVERLAPPED             NativeOverlappedObject;
        };
        struct xOverlappedIoBuffer
        : ::xel::xNonCopyable
        {
            bool                  DeleteMark;
            ssize_t               ReferenceCount;
            xOverlappedObject     ReadObject;
            ubyte                 ReadBuffer[InternalReadBufferSize];
            size_t                UnprocessedDataSize;
            xOverlappedObject     WriteObject;
            xPacketBufferChain    WriteBufferChain;
        };
        xOverlappedIoBuffer * _IoBufferPtr = nullptr;

        X_PRIVATE_STATIC_MEMBER xOverlappedIoBuffer * CreateOverlappedObject();
        X_PRIVATE_STATIC_MEMBER void    RetainOverlappedObject(xOverlappedIoBuffer * IoBufferPtr) { ++IoBufferPtr->ReferenceCount; }
        X_PRIVATE_STATIC_MEMBER ssize_t ReleaseOverlappedObject(xOverlappedIoBuffer * IoBufferPtr);

    #endif
    };

}
