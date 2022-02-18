#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include <string_view>
#include <string>

#include "./IoContext.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xSocks5Client
    : xNonCopyable
    {
    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, const std::string & Username, const std::string & Password);
        ZEC_API_MEMBER void Clean();

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        enum : uint8_t
        {
            Inited, Challenging, AuthPosed, Authorized, Connecting, Connected, Closed
        } _Phase;

        xIoContext *                  _IoContextPtr {};
        std::string                   _Username {};
        std::string                   _Password {};
        xNetAddress                   _ProxyAddress {};
        uint16_t                      _ProxyPort {};

        alignas(max_align_t) ubyte    _Dummy[64];
        friend class __detail__::IOUtil;
    };

    class xHttp2Socks5
    : xNonCopyable
    {

    };

}
