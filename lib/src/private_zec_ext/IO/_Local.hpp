#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/Resolver.hpp>
#include <zec_ext/IO/Socks5Proxy.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;

using boost::system::error_code;

ZEC_NS
{
    using xNativeIoContext = asio::io_context;
    using xNativeIoContextHolder = xHolder<xNativeIoContext>;
    static inline xNativeIoContextHolder & NativeIoContextHolderRef(void * NativePtr) { return *static_cast<xNativeIoContextHolder*>(NativePtr); }

    using xNativeTcpResolver = asio::ip::tcp::resolver;
    using xNativeTcpResolverHolder = xHolder<xNativeTcpResolver>;
    static inline xNativeTcpResolverHolder & NativeTcpResolverHolderRef(void * NativePtr) { return *static_cast<xNativeTcpResolverHolder*>(NativePtr); }

    using xNativeTcpSocket = asio::ip::tcp::socket;
    using xNativeTcpSocketHolder = xHolder<xNativeTcpSocket>;
    static inline xNativeTcpSocketHolder & NativeTcpSocketHolderRef(void * NativePtr) { return *static_cast<xNativeTcpSocketHolder*>(NativePtr); }

    namespace __detail__ {
        class IOUtil {
        public:

            static inline xNativeIoContext *    Native(xIoContext * IoContextPtr)  { return NativeIoContextHolderRef(IoContextPtr->Native()).Get(); }
            static inline xNativeTcpResolver *  Native(xTcpResolver * ResolverPtr)  { return NativeTcpResolverHolderRef(ResolverPtr->Native()).Get(); }
            static inline xNativeTcpSocket *    Native(xSocks5Client * Sock5ClientPtr)  { return NativeTcpSocketHolderRef(Sock5ClientPtr->Native()).Get(); }

        private:
            static_assert(sizeof(xIoContext::_Dummy) >= sizeof(xNativeIoContextHolder));
            static_assert(alignof(xIoContext::_Dummy) >= alignof(xNativeIoContextHolder));
            static_assert(!(alignof(xIoContext::_Dummy) % alignof(xNativeIoContextHolder)));

            static_assert(sizeof(xTcpResolver::_Dummy) >= sizeof(xNativeTcpResolverHolder));
            static_assert(alignof(xTcpResolver::_Dummy) >= alignof(xNativeTcpResolverHolder));
            static_assert(!(alignof(xTcpResolver::_Dummy) % alignof(xNativeTcpResolverHolder)));

            static_assert(sizeof(xSocks5Client::_Dummy) >= sizeof(xNativeTcpSocketHolder));
            static_assert(alignof(xSocks5Client::_Dummy) >= alignof(xNativeTcpSocketHolder));
            static_assert(!(alignof(xSocks5Client::_Dummy) % alignof(xNativeTcpSocketHolder)));
        };
    }
    using IOUtil = __detail__::IOUtil;

}
