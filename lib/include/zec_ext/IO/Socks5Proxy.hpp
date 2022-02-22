#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include <string_view>
#include <string>

#include "./IoContext.hpp"
#include "./TcpConnection.hpp"

ZEC_NS
{

    class xSocks5Client
    : xNonCopyable
    , xTcpConnection::iListener
    {
    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, const std::string & Username, const std::string & Password);
        ZEC_API_MEMBER void Clean();

    private:
        enum : uint8_t
        {
            Inited, Challenging, AuthPosed, Authorized, Connecting, Connected, Closed
        } _Phase;

        xTcpConnection                    _TcpConnection {};
        std::string                   _Username {};
        std::string                   _Password {};
    };

    class xHttp2Socks5
    : xNonCopyable
    {

    };

}
