#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/Resolver.hpp>
#include <zec_ext/IO/TcpClient.hpp>
#include <zec_ext/IO/WebSocket.hpp>

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
    using xNativeWebSocket = websocket::stream<tcp::socket>;

    using xNativeTcpResolverHolder = xHolder<xNativeTcpResolver>;
    ZEC_STATIC_INLINE xNativeTcpResolverHolder & NativeTcpResolverHolderRef(void * NativePtr) { return *static_cast<xNativeTcpResolverHolder*>(NativePtr); }

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
            ZEC_STATIC_INLINE xNativeIoContext *    Native(xIoContext * IoContextPtr)  { return &IoContextPtr->Native().As<xNativeIoContext>(); }
            ZEC_STATIC_INLINE xNativeTcpResolver *  Native(xTcpResolver * ResolverPtr)  { return NativeTcpResolverHolderRef(ResolverPtr->Native()).Get(); }
        };
    }
    using IOUtil = __detail__::IOUtil;

}
