#pragma once

#include <xel/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./NetBase.hpp"
#include "./Packet.hpp"
#include "./PacketBuffer.hpp"

X_NS
{

    class xTcpSocketContext;
    class xTcpConnection;
    class xTcpServer;

    class xTcpConnection
    : xAbstract
    {
    public:
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
        X_API_MEMBER xTcpConnection();
        X_API_MEMBER ~xTcpConnection();

        X_API_MEMBER bool Init(xIoHandle NativeHandle, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint16_t Port, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
        X_API_MEMBER bool GracefulClose();  // return value: true: immediately closed, false pending writes
        X_API_MEMBER void Clean();

        X_API_MEMBER xNetAddress GetRemoteAddress() const;
        X_API_MEMBER xNetAddress GetLocalAddress() const;

        X_API_MEMBER void ResizeSendBuffer(size_t Size);
        X_API_MEMBER void ResizeReceiveBuffer(size_t Size);

        X_API_MEMBER size_t GetPendingWriteBlockCount() const;
        X_INLINE bool IsActive() const { return _SocketPtr; }

        struct xAudit {
            size64_t ReadSize  = 0;
            size64_t WriteSize = 0;
        };
        X_API_MEMBER xAudit GetAudit();
        X_API_MEMBER xAudit StealAudit();

        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        X_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        X_API_MEMBER void SuspendReading();
        X_API_MEMBER void ResumeReading();

    private:
        X_PRIVATE_MEMBER void OnConnected();
        X_PRIVATE_MEMBER void OnError();
        X_PRIVATE_MEMBER void DoRead();

    private:
        xTcpSocketContext *           _SocketPtr = nullptr;
    };

}
