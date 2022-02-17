#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <zec_ext/IO/IoContext.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;

ZEC_NS
{

    using xNativeIoContextHolder = xHolder<asio::io_context>;
    static constexpr const size_t ContextSize = sizeof(asio::io_context);
    static constexpr const size_t HolderSize = sizeof(xNativeIoContextHolder);

    namespace __detail__ {
        class IOUtil {
        public:

        private:
            static_assert(sizeof(xIoContext::_Dummy) >= sizeof(xNativeIoContextHolder));
            static_assert(alignof(xIoContext::_Dummy) >= alignof(xNativeIoContextHolder));
            static_assert(!(alignof(xIoContext::_Dummy) % alignof(xNativeIoContextHolder)));
        };
    }

    ZEC_STATIC_INLINE xNativeIoContextHolder& GetHolderRef(xIoContext * IoContextPtr) { return *static_cast<xNativeIoContextHolder *>(IoContextPtr->Native()); }

}
