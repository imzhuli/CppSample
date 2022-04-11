#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./NetBase.hpp"
#include "./Packet.hpp"
#include "./PacketBuffer.hpp"

ZEC_NS
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
        ZEC_API_MEMBER xTcpConnection();
        ZEC_API_MEMBER ~xTcpConnection();

        ZEC_API_MEMBER bool Init(xIoHandle NativeHandle, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
        ZEC_API_MEMBER bool GracefulClose();  // return value: true: immediately closed, false pending writes
        ZEC_API_MEMBER void Clean();

        ZEC_API_MEMBER xNetAddress GetRemoteAddress() const;
        ZEC_API_MEMBER xNetAddress GetLocalAddress() const;

        ZEC_API_MEMBER void ResizeSendBuffer(size_t Size);
        ZEC_API_MEMBER void ResizeReceiveBuffer(size_t Size);

        ZEC_API_MEMBER size_t GetPendingWriteBlockCount() const;
        ZEC_INLINE bool IsActive() const { return _SocketPtr; }

        struct xAudit {
            size64_t ReadSize  = 0;
            size64_t WriteSize = 0;
        };
        ZEC_API_MEMBER xAudit GetAudit();
        ZEC_API_MEMBER xAudit StealAudit();

        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void SuspendReading();
        ZEC_API_MEMBER void ResumeReading();

    private:
        ZEC_PRIVATE_MEMBER void OnConnected();
        ZEC_PRIVATE_MEMBER void OnError();
        ZEC_PRIVATE_MEMBER void DoRead();

    private:
        xTcpSocketContext *           _SocketPtr = nullptr;
    };

}
