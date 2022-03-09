#pragma once
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
    :  xNonCopyable
    {
    public:
        ZEC_API_MEMBER xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port);
        ZEC_API_MEMBER xTcpSocketContext(xIoHandle Handle);
        ZEC_API_MEMBER ~xTcpSocketContext();

        xTcpSocket                    _Socket;
        xPacketBufferQueue            _WritePacketBufferQueue;
        size_t                        _WriteDataSize = 0;
        ubyte                         _ReadBuffer[MaxPacketSize + 1];
        size_t                        _ReadDataSize = 0;

        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void   SuspendReading();
        ZEC_API_MEMBER void   Close();

    private:
        xPacketBuffer * NewWriteBuffer() { return new xPacketBuffer(); }
        void DeleteWriteBuffer(xPacketBuffer * BufferPtr) { delete BufferPtr; }

        ZEC_API_MEMBER void OnExpired();
        ZEC_API_MEMBER void OnConnected();
        ZEC_API_MEMBER void DoRead();
        ZEC_API_MEMBER void DoFlush();
        ZEC_API_MEMBER void OnError();

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
            eReadToBeSuspended,
            eReadSuspended,
        } _ReadState;
    };
    using xSharedTcpSocketContextPtr = std::shared_ptr<xTcpSocketContext>;

}
