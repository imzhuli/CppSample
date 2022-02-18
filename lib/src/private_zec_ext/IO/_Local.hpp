#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/Resolver.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;

using boost::system::error_code;

ZEC_NS
{
    using xNativeIoContext = asio::io_context;
    using xNativeResolver = asio::ip::tcp::resolver;

    using xNativeIoContextHolder = xHolder<xNativeIoContext>;
    using xNativeResolverHolder = xHolder<xNativeResolver>;

    static inline xNativeIoContextHolder &   NativeIoContextHolderRef(void * NativePtr) { return *static_cast<xNativeIoContextHolder*>(NativePtr); }
    static inline xNativeResolverHolder &    NativeResolverHolderRef(void * NativePtr) { return *static_cast<xNativeResolverHolder*>(NativePtr); }

    namespace __detail__ {
        class IOUtil {
        public:

            static inline xNativeIoContext * Native(xIoContext * IoContextPtr)  { return NativeIoContextHolderRef(IoContextPtr->Native()).Get(); }
            static inline xNativeResolver *  Native(xResolver * ResolverPtr)  { return NativeResolverHolderRef(ResolverPtr->Native()).Get(); }

        private:
            static_assert(sizeof(xIoContext::_Dummy) >= sizeof(xNativeIoContextHolder));
            static_assert(alignof(xIoContext::_Dummy) >= alignof(xNativeIoContextHolder));
            static_assert(!(alignof(xIoContext::_Dummy) % alignof(xNativeIoContextHolder)));

            static_assert(sizeof(xResolver::_Dummy) >= sizeof(xNativeResolver));
            static_assert(alignof(xResolver::_Dummy) >= alignof(xNativeResolver));
            static_assert(!(alignof(xResolver::_Dummy) % alignof(xNativeResolver)));
        };
    }
    using IOUtil = __detail__::IOUtil;

}
