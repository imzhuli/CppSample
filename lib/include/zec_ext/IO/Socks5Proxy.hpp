#pragma once

#include <zec/Common.hpp>
#include <asio/asio.hpp>
#include <atomic>
#include <string_view>
#include <string>

ZEC_NS
{

    class iProxyAuth
    {
        virtual bool CheckUserPass(const std::string_view & Username, const std::string_view & Password) = 0;
    };

    class xSocks5Client
    : xNonCopyable
    {
    public:
        bool Init(asio::io_context * IoContextPtr, const asio::ip::tcp::endpoint & TargetAddress, const std::string_view & Username, const std::string_view & Password);
        void Clean();

    protected:
        virtual void ConnectionHandler(const asio::error_code& ErrorCode);

    private:
        asio::io_context *              _IoContextPtr {};
        xHolder<asio::ip::tcp::socket>  _SocketHolder {};
        std::string                     _Username {};
        std::string                     _Password {};
    };

    class xSocks5Proxy
    : xNonCopyable
    {
    public:
        bool Init(asio::io_context * IoContextPtr, const asio::ip::tcp::endpoint & BindAddress, iProxyAuth * AuthCallback);
        void Clean();

    private:
        asio::io_context *                 _IoContextPtr = {};
        asio::ip::address                  _Address = {};
        xHolder<asio::ip::tcp::acceptor>   _AcceptorHolder = {};
        iProxyAuth *                       _AuthCallback = {};
    };

}