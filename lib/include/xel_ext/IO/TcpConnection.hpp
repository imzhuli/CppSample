#pragma once

#include <xel/Common.hpp>
#include "./IoContext.hpp"
#include "./NetAddress.hpp"
#include "./Packet.hpp"
#include "./PacketBuffer.hpp"
#include <atomic>

X_NS
{

    class xTcpConnection;
    class xTcpServer;

    class xTcpConnection
    : public iBufferedIoReactor
	, protected xIoContext::xDeferredCallbackNode
    , xAbstract
    {
    public:
        enum struct eStatus
        {
            Unspecified,
            Connecting,
            Connected,
            Closing,
        };

        struct iListener
        {
            // callback on connected, normally this is not needed to be handled
            virtual void   OnConnected(xTcpConnection * TcpConnectionPtr)  { }
            /***
             * OnReceivedData:
             * called when there is some data in,
             * @return consumed bytes
             * */
            virtual size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) { return DataSize; }
            virtual void   OnFlush(xTcpConnection * TcpConnectionPtr) {}
            virtual void   OnPeerClose(xTcpConnection * TcpConnectionPtr)  {}
            virtual void   OnError(xTcpConnection * TcpConnectionPtr) {}
        };

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
        X_API_MEMBER void Clean();

        X_API_MEMBER xNetAddress GetRemoteAddress() const;
        X_API_MEMBER xNetAddress GetLocalAddress() const;

        X_API_MEMBER void ResizeSendBuffer(size_t Size);
        X_API_MEMBER void ResizeReceiveBuffer(size_t Size);

        #ifndef X_SYSTEM_WINDOWS
        X_INLINE bool HasPendingWrites() const { return !_FlushFlag; }
        #else // X_SYSTEM_WINDOWS
        X_INLINE bool HasPendingWrites() const {
            assert(_IoBufferPtr);
            return !_IoBufferPtr->FlushFlag;
        }
        #endif

        // X_API_MEMBER size_t  GetPendingWriteBlockCount() const;
        // X_API_MEMBER bool    GracefulClose();  /* return value: true: immediately closed, false pending writes */

        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        X_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        X_API_MEMBER void SuspendReading();
        X_API_MEMBER void ResumeReading();

    protected:
        X_PRIVATE_MEMBER void OnIoEventError() override {
            if (_Status < eStatus::Closing) {
                _Status = eStatus::Closing;
                _ListenerPtr->OnError(this);
            }
        }

    #if defined (X_SYSTEM_DARWIN)
        X_PRIVATE_MEMBER void OnIoEventInReady() override;
        X_PRIVATE_MEMBER void OnIoEventOutReady() override;
        X_PRIVATE_MEMBER void TrySendData();
        X_PRIVATE_MEMBER void EnableReadingTrigger();
        X_PRIVATE_MEMBER void DisableReadingTrigger();
        X_PRIVATE_MEMBER void EnableWritingTrigger();
        X_PRIVATE_MEMBER void DisableWritingTrigger();
        bool _RequireOutputEvent X_DEBUG_INIT(false);
        bool _FlushFlag X_DEBUG_INIT(false);
    #endif

    #if defined (X_SYSTEM_LINUX)
        X_PRIVATE_MEMBER void OnIoEventInReady() override;
        X_PRIVATE_MEMBER void OnIoEventOutReady() override;
        X_PRIVATE_MEMBER void TrySendData();
        X_PRIVATE_MEMBER void UpdateEventTrigger();
        bool _RequireOutputEvent X_DEBUG_INIT(false);
        bool _FlushFlag X_DEBUG_INIT(false);
    #endif

    #if defined (X_SYSTEM_WINDOWS)
        X_API_MEMBER void OnIoEventInReady() override;
        X_API_MEMBER void OnIoEventOutReady() override;
        X_API_MEMBER void OnDeferredCallback() override;
        X_PRIVATE_MEMBER void TryRecvData();
        X_PRIVATE_MEMBER void TrySendData();
    #endif

    private:
        xSocket        _Socket X_DEBUG_INIT(InvalidSocket);
        eStatus        _Status X_DEBUG_INIT(eStatus::Unspecified);

        xIoContext *   _IoContextPtr X_DEBUG_INIT(nullptr);
        iListener *    _ListenerPtr X_DEBUG_INIT(nullptr);

        bool           _SuspendReading X_DEBUG_INIT(false);
    };

}
