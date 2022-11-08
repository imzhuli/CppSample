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
            Closed,
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
            virtual size_t  OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) { return 0; }
            virtual void    OnPeerClose(xTcpConnection * TcpConnectionPtr)  {}
            virtual void    OnError(xTcpConnection * TcpConnectionPtr) {}
        };

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint16_t Port, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
        X_API_MEMBER bool GracefulClose();  // return value: true: immediately closed, false pending writes
        X_API_MEMBER void Clean();

        X_API_MEMBER xNetAddress GetRemoteAddress() const;
        X_API_MEMBER xNetAddress GetLocalAddress() const;

        X_API_MEMBER void ResizeSendBuffer(size_t Size);
        X_API_MEMBER void ResizeReceiveBuffer(size_t Size);
        
        X_API_MEMBER size_t GetPendingWriteBlockCount() const;

        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        X_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        X_API_MEMBER void SuspendReading();
        X_API_MEMBER void ResumeReading();

    protected:
    #if defined (X_SYSTEM_WINDOWS)
        X_API_MEMBER eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) override {
            return OverlappedPtr == &_ReadOverlappedObject ? eIoEventType::InReady : 
                    (OverlappedPtr == &_WriteOverlappedObject ? eIoEventType::OutReady : eIoEventType::Error);
        }
        X_API_MEMBER void OnIoEventInReady() override;
        X_API_MEMBER void OnIoEventOutReady() override;
        X_API_MEMBER void TryRecvData(size_t SkipSize = 0);
        X_API_MEMBER void TrySendData();
    #endif

    private:
        X_PRIVATE_MEMBER void OnConnected();
        X_PRIVATE_MEMBER void OnError();
        X_PRIVATE_MEMBER void DoRead();

    private:
        xSocket        _Socket X_DEBUG_INIT(InvalidSocket);
        eStatus        _Status X_DEBUG_INIT(eStatus::Unspecified);
        xIoContext *   _IoContextPtr X_DEBUG_INIT(nullptr);
        iListener *    _ListenerPtr X_DEBUG_INIT(nullptr);
    };

}
