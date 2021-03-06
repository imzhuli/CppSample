#pragma once
#include "./_Local.hpp"
#include <zec/Memory.hpp>
#include <zec_ext/IO/TcpConnection.hpp>

ZEC_NS
{
    using xTcpSocket = tcp::socket;
    
    class xTcpSocketContext
    : public xRetainable
    , xNonCopyable
    {
    public:
        ZEC_API_MEMBER xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, xTcpConnection * TcpConnectionPtr, xTcpConnection::iListener * ListenerPtr);
        ZEC_API_MEMBER xTcpSocketContext(xIoHandle Handle, xTcpConnection * TcpConnectionPtr, xTcpConnection::iListener * ListenerPtr);
        ZEC_API_MEMBER ~xTcpSocketContext();

        ZEC_INLINE tcp::endpoint GetLocalAddress() const { return _Socket.local_endpoint(X2Ref(xAsioError{})); }
        ZEC_INLINE tcp::endpoint GetRemoteAddress() const { return _Socket.remote_endpoint(X2Ref(xAsioError{})); }

        ZEC_API_MEMBER void ResizeSendBuffer(size_t Size) { asio::socket_base::send_buffer_size Option((int)Size); _Socket.set_option(Option); }
        ZEC_API_MEMBER void ResizeReceiveBuffer(size_t Size) { asio::socket_base::receive_buffer_size Option((int)Size); _Socket.set_option(Option); }

        ZEC_INLINE bool IsReadingSusppended() { return eReading != _ReadState; }

        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void   SuspendReading();
        ZEC_API_MEMBER void   ResumeReading();
        ZEC_API_MEMBER bool   GracefulClose();
        ZEC_API_MEMBER void   Close();

        ZEC_INLINE  size64_t GetTotalReadSize() { return _TotalReadSize; }
        ZEC_INLINE  size64_t GetTotalWriteSize() { return _TotalWriteSize; }
        ZEC_INLINE  size64_t StealTotalReadSize() { return Steal(_TotalReadSize); }
        ZEC_INLINE  size64_t StealTotalWriteSize() { return Steal(_TotalWriteSize); }
        ZEC_INLINE  size_t   GetPendingWriteBlocks() const { return _WritePacketBufferQueue.GetSize(); }

    private:
        xPacketBuffer * NewWriteBuffer() { return new xPacketBuffer(); }
        void DeleteWriteBuffer(xPacketBuffer * BufferPtr) { delete BufferPtr; }

        ZEC_API_MEMBER void OnExpired();
        ZEC_API_MEMBER void OnConnected();
        ZEC_API_MEMBER void DoRead();
        ZEC_API_MEMBER void DoReadCallback();
        ZEC_API_MEMBER void DoFlush();
        ZEC_API_MEMBER void DoClose();
        ZEC_API_MEMBER void OnError();

        xTcpSocket                    _Socket;
        xPacketBufferQueue            _WritePacketBufferQueue;
        size_t                        _WriteDataSize = 0;
        ubyte                         _ReadBuffer[MaxPacketSize + 1];
        size_t                        _ReadDataSize = 0;
        xReentryFlag                  _ReadCallbackEntry;
        xTcpConnection::iListener *   _ListenerPtr;
        xTcpConnection *              _ListenerContextPtr;

        size_t                        _TotalReadSize = 0;
        size_t                        _TotalWriteSize = 0;

        enum eConnectionState {
            eUnspecified,
            eConnecting,
            eConnected,
            eConnectionClosing,
            eConnectionClosed,
            eConnectionError,
        } _ConnectionState = eUnspecified;

        enum eReadState {
            eReading,
            eReadSuspended,
        } _ReadState = eReading;
    };
    using xSharedTcpSocketContextPtr = std::shared_ptr<xTcpSocketContext>;

}
