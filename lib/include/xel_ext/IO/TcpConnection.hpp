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
            /***
             * Callbacks:
             *
             * OnConnected: when connection is made.
             * OnData: when some reading is made
             * OnFlush: when all pending writing is done.
             * OnPeerClose: when closed by peer (0 == read())
             * OnError: when some error detected by system event system
             *
             * Causion: !!!
             *
             * Please not that not all operations are asynchronously done,
             * Connection could be made or fail immediately to a local address, or an invalid address.
             * so OnConnected is only called on asynchronous call.
             * The same applies to OnFlush. Though technically it's possible to call onflush every time the sending queue becomes empty,
             * but to do that, it bacome also very important to prevent callback reentry issue, like OnFlush->PostData->OnFlush, which is rere in most cases.
             * So OnFlush is only called when an asynchronous call is done.
             *
             * Practise:
             * If some just wants to send Data immediately when connection is established, just call PostData after Connection->Init, data is buffered, and sent
             * when connection is established
             * But in case establish connection took some long time, and data is quite droppable, check connection state by IsConnected() after calling init, and
             * decide to PostData or not
             *
             * It's similar to OnFlush.
             * There are cases that some wants to send and close connection, but data is large, and a graceful closing is expected.
             * PostData and check by calling HasPendingWrites, if there is no pending writes, it's safe to call Clean() to close connection,
             * or waits for OnFlush.
             *
            */
            // callback on connected, normally this is not needed to be handled
            virtual void   OnConnected(xTcpConnection * TcpConnectionPtr)  { }
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

        X_INLINE bool IsConnected() const { return _Status == eStatus::Connected; }
        X_INLINE bool HasPendingWrites() const { return _HasPendingWriteFlag; }

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
            assert(_Status < eStatus::Closing); // prevent re-entry
            _Status = eStatus::Closing;
            _ListenerPtr->OnError(this);
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
    #endif

    #if defined (X_SYSTEM_LINUX)
        X_PRIVATE_MEMBER void OnIoEventInReady() override;
        X_PRIVATE_MEMBER void OnIoEventOutReady() override;
        X_PRIVATE_MEMBER void TrySendData();
        X_PRIVATE_MEMBER void UpdateEventTrigger();
        bool _RequireOutputEvent X_DEBUG_INIT(false);
    #endif

    #if defined (X_SYSTEM_WINDOWS)
        X_API_MEMBER void OnIoEventInReady() override;
        X_API_MEMBER void OnIoEventOutReady() override;
        X_PRIVATE_MEMBER void TryRecvData();
        X_PRIVATE_MEMBER void TrySendData();
    #endif

    private:
        xSocket        _Socket X_DEBUG_INIT(InvalidSocket);
        eStatus        _Status X_DEBUG_INIT(eStatus::Unspecified);

        xIoContext *   _IoContextPtr X_DEBUG_INIT(nullptr);
        iListener *    _ListenerPtr X_DEBUG_INIT(nullptr);

        bool           _SuspendReading X_DEBUG_INIT(false);
        bool           _HasPendingWriteFlag X_DEBUG_INIT(false);
    };

}
