#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./PacketData.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }
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
            virtual size_t  OnReceiveData(xTcpConnection * TcpConnectionPtr, const void * DataPtr, size_t DataSize) { return 0; }
            virtual void    OnPeerClose(xTcpConnection * TcpConnectionPtr)  {}
            virtual void    OnError(xTcpConnection * TcpConnectionPtr) {}
        };

    public:
        ZEC_API_MEMBER xTcpConnection();
        ZEC_API_MEMBER ~xTcpConnection();

        ZEC_API_MEMBER bool Init(xIoNativeHandle NativeHandle, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr);
        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void Clean();

    protected:
        ZEC_API_MEMBER virtual xPacketBuffer * NewWriteBuffer() { return new xPacketBuffer(); }
        ZEC_API_MEMBER virtual void DeleteWriteBuffer(xPacketBuffer * BufferPtr) { delete BufferPtr; }

    private:
        ZEC_PRIVATE_MEMBER void OnConnected();
        ZEC_PRIVATE_MEMBER void OnError();
        ZEC_PRIVATE_MEMBER void DoRead();
        ZEC_PRIVATE_MEMBER void DoFlush();

    private:
        iListener *                   _ListenerPtr = nullptr;
        xPacketBufferQueue            _WritePacketBufferQueue;
        size_t                        _WriteDataSize = 0;

        ubyte                         _ReadBuffer[MaxPacketSize];
        size_t                        _ReadDataSize = 0;

        xDummy<16>                    _NativeContext;
        bool                          _Connected = false;

        friend class __detail__::IOUtil;
        friend class xTcpServer;
    };

}
