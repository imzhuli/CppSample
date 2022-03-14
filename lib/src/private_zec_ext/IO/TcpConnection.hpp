#pragma once
#include "_Local.hpp"
#include <zec/Memory.hpp>
#include <zec_ext/IO/TcpConnection.hpp>

ZEC_NS
{
    using xTcpSocket = tcp::socket;
    using xTcpEndpoint = tcp::endpoint;
    ZEC_STATIC_INLINE xTcpEndpoint MakeTcpEndpoint(const xNetAddress & Address) {
        if (Address.IsV4()) {
            static_assert(std::tuple_size<ip::address_v4::bytes_type>() == 4);
            ip::address_v4::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv4, 4);
            return { ip::make_address_v4(Bytes), Address.Port };
        } else if (Address.IsV6()) {
            static_assert(std::tuple_size<ip::address_v6::bytes_type>() == 16);
            ip::address_v6::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv6, 16);
            return { ip::make_address_v6(Bytes), Address.Port };
        }
        return {};
    }

    class xTcpSocketContext
    : public xRetainable
    , xNonCopyable
    {
    public:
        ZEC_API_MEMBER xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address);
        ZEC_API_MEMBER xTcpSocketContext(xIoHandle Handle);
        ZEC_API_MEMBER ~xTcpSocketContext();

        ZEC_INLINE tcp::endpoint GetLocalAddress() const { return _Socket.local_endpoint(); }
        ZEC_INLINE tcp::endpoint GetRemoteAddress() const { return _Socket.remote_endpoint(); }

        ZEC_API_MEMBER void ResizeSendBuffer(size_t Size) { asio::socket_base::send_buffer_size Option(Size); _Socket.set_option(Option); }
        ZEC_API_MEMBER void ResizeReceiveBuffer(size_t Size) { asio::socket_base::receive_buffer_size Option(Size); _Socket.set_option(Option); }

        ZEC_INLINE void BindListener(xTcpConnection::iListener * ListenerPtr, xTcpConnection * ListenerContextPtr) { _ListenerPtr = ListenerPtr; _ListenerContextPtr = ListenerContextPtr; }
        ZEC_INLINE bool IsReadingSusppended() { return eReading != _ReadState; }

        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void   SuspendReading();
        ZEC_API_MEMBER void   ResumeReading();
        ZEC_API_MEMBER void   Close();

        ZEC_INLINE  size64_t GetTotalReadSize() { return _TotalReadSize; }
        ZEC_INLINE  size64_t GetTotalWriteSize() { return _TotalWriteSize; }

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
        } _ReadState;
    };
    using xSharedTcpSocketContextPtr = std::shared_ptr<xTcpSocketContext>;

}
