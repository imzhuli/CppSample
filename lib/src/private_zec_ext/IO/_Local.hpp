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
using xAsioBuffer = boost::asio::mutable_buffer;

ZEC_NS
{
    using xNativeIoContext = asio::io_context;
    using xNativeIoContextHolder = xHolder<xNativeIoContext>;
    ZEC_STATIC_INLINE xNativeIoContextHolder & NativeIoContextHolderRef(void * NativePtr) { return *static_cast<xNativeIoContextHolder*>(NativePtr); }

    using xNativeTcpResolver = tcp::resolver;
    using xNativeTcpResolverHolder = xHolder<xNativeTcpResolver>;
    ZEC_STATIC_INLINE xNativeTcpResolverHolder & NativeTcpResolverHolderRef(void * NativePtr) { return *static_cast<xNativeTcpResolverHolder*>(NativePtr); }

    using xNativeTcpSocket = tcp::socket;
    using xNativeTcpSocketHolder = xHolder<xNativeTcpSocket>;
    ZEC_STATIC_INLINE xNativeTcpSocket * NativeTcpSocket(void * NativePtr) { return static_cast<xNativeTcpSocketHolder*>(NativePtr)->Get(); }
    ZEC_STATIC_INLINE xNativeTcpSocketHolder & NativeTcpSocketHolderRef(void * NativePtr) { return *static_cast<xNativeTcpSocketHolder*>(NativePtr); }

    using xNativeWebSocket = websocket::stream<tcp::socket>;
    using xNativeWebSocketHolder = xHolder<xNativeWebSocket>;
    ZEC_STATIC_INLINE xNativeWebSocket * NativeWebSocket(void * NativePtr) { return static_cast<xNativeWebSocketHolder*>(NativePtr)->Get(); }
    ZEC_STATIC_INLINE xNativeWebSocketHolder & NativeWebSocketHolderRef(void * NativePtr) { return *static_cast<xNativeWebSocketHolder*>(NativePtr); }

    ZEC_STATIC_INLINE tcp::endpoint MakeAddress(const xNetAddress & Address, uint16_t Port) {
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
            ZEC_STATIC_INLINE xNativeIoContext *    Native(xIoContext * IoContextPtr)  { return NativeIoContextHolderRef(IoContextPtr->Native()).Get(); }
            ZEC_STATIC_INLINE xNativeTcpResolver *  Native(xTcpResolver * ResolverPtr)  { return NativeTcpResolverHolderRef(ResolverPtr->Native()).Get(); }
            ZEC_STATIC_INLINE xNativeTcpSocket *    Native(xTcpClient * TcpClientPtr)  { return NativeTcpSocketHolderRef(TcpClientPtr->Native()).Get(); }

        private:
            static_assert(sizeof(xIoContext::_Dummy) >= sizeof(xNativeIoContextHolder));
            static_assert(alignof(xIoContext::_Dummy) >= alignof(xNativeIoContextHolder));
            static_assert(!(alignof(xIoContext::_Dummy) % alignof(xNativeIoContextHolder)));

            static_assert(sizeof(xTcpResolver::_Dummy) >= sizeof(xNativeTcpResolverHolder));
            static_assert(alignof(xTcpResolver::_Dummy) >= alignof(xNativeTcpResolverHolder));
            static_assert(!(alignof(xTcpResolver::_Dummy) % alignof(xNativeTcpResolverHolder)));

            static_assert(sizeof(xTcpClient::_Dummy) >= sizeof(xNativeTcpSocketHolder));
            static_assert(alignof(xTcpClient::_Dummy) >= alignof(xNativeTcpSocketHolder));
            static_assert(!(alignof(xTcpClient::_Dummy) % alignof(xNativeTcpSocketHolder)));
        };
    }
    using IOUtil = __detail__::IOUtil;

}
