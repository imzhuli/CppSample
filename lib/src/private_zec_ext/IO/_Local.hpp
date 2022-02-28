#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/TcpConnection.hpp>

namespace asio        = boost::asio;
namespace beast       = boost::beast;
namespace ip          = boost::asio::ip;
namespace websocket   = boost::beast::websocket;

using tcp   = boost::asio::ip::tcp;
using udp   = boost::asio::ip::udp;

using xAsioError = boost::system::error_code;
using xAsioConstBuffer = boost::asio::const_buffer;
using xAsioMutableBuffer = boost::asio::mutable_buffer;
using xBeastDynamicBuffer = boost::beast::multi_buffer;

ZEC_NS
{

    using xNativeIoContext = asio::io_context;
    using xNativeTcpResolver = tcp::resolver;
    using xNativeTcpSocket = tcp::socket;
    using xNativeTcpAcceptor = tcp::acceptor;
    using xNativeWebSocket = websocket::stream<tcp::socket>;

    class xTcpSocketContext
    : public xIoContext::xExpiringNode
    , xNonCopyable
    {
    public:
        xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port);
        xTcpSocketContext(xIoNativeHandle Handle);
        ~xTcpSocketContext();

        tcp::socket Socket;
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
    ZEC_STATIC_INLINE xTcpEndpoint MakeEndpoint(const xNetAddress & Address, uint16_t Port) {
        if (Address.IsV4()) {
            static_assert(std::tuple_size<ip::address_v4::bytes_type>() == 4);
            ip::address_v4::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv4, 4);
            return { ip::make_address_v4(Bytes), Port };
        } else if (Address.IsV6()) {
            static_assert(std::tuple_size<ip::address_v6::bytes_type>() == 16);
            ip::address_v6::bytes_type Bytes;
            memcpy(Bytes.data(), Address.Ipv6, 16);
            return { ip::make_address_v6(Bytes), Port };
        }
        return {};
    }

    namespace __detail__ {
        class IOUtil {
        public:
            ZEC_STATIC_INLINE xNativeIoContext *    Native(xIoContext * IoContextPtr)  { return &IoContextPtr->_Native.As<xNativeIoContext>(); }
        };
    }
    using IOUtil = __detail__::IOUtil;

}
