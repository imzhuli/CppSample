#pragma once

#include <zec/Common.hpp>
#include <asio/asio.hpp>
#include <atomic>
#include <string_view>

ZEC_NS
{

    class iProxyAuth
    {
        virtual bool CheckUserPass(const std::string_view & Username, const std::string_view & Password) = 0;
    };

    class xSocks5Proxy
    : xNonCopyable
    {
    public:
        bool Init(asio::io_context * IoContextPtr, const asio::ip::tcp::endpoint & BindAddress, iProxyAuth * AuthCallback);
        void Clean();
        void Run();
        void Stop();

    private:
        std::atomic_bool                   _StopFlag = {};
        iProxyAuth *                       _AuthCallback = {};
        asio::io_context *                 _IoContextPtr = {};
        asio::ip::address                  _Address = {};
        xHolder<asio::ip::tcp::acceptor>   _AcceptorHolder = {};
    };

}