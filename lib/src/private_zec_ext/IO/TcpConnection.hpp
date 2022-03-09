#pragma once
#include <zec_ext/IO/TcpConnection.hpp>

ZEC_NS
{

    class xTcpSocketContext
    :  xNonCopyable
    {
    public:
        xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port);
        xTcpSocketContext(xIoHandle Handle);
        ~xTcpSocketContext();

        tcp::socket                   _Socket;
        xPacketBufferQueue            _WritePacketBufferQueue;
        size_t                        _WriteDataSize = 0;
        ubyte                         _ReadBuffer[MaxPacketSize + 1];
        size_t                        _ReadDataSize = 0;

        size_t PostData(const void * DataPtr, size_t DataSize);
        void   Close();

    private:
        xPacketBuffer * NewWriteBuffer() { return new xPacketBuffer(); }
        void DeleteWriteBuffer(xPacketBuffer * BufferPtr) { delete BufferPtr; }

        void OnExpired();
        void OnConnected();
        void DoRead();
        void DoFlush();
        void OnError();
        bool _Connected = false;
        bool _Closing = false;
        bool _Error = false;
    };
    using xSharedTcpSocketContextPtr = std::shared_ptr<xTcpSocketContext>;

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

}
