#include <zec_ext/Net/Socks5Proxy.hpp>

ZEC_NS
{

    bool xSocks5Client::Init(asio::io_context * IoContextPtr, const asio::ip::tcp::endpoint & TargetAddress, const std::string_view & Username, const std::string_view & Password)
    {
        try {
            _SocketHolder.New(*IoContextPtr);
            _Username = Username;
            _Password = Password;
            _SocketHolder->non_blocking(true);
            _SocketHolder->async_connect(TargetAddress, [this](const asio::error_code& ErrorCode){ConnectionHandler(ErrorCode);});
        }
        catch (...) {
            if (_SocketHolder.IsValid()) {
                _SocketHolder.Delete();
            }
            Reset(_IoContextPtr);
            Reset(_Username);
            Reset(_Password);
            return false;
        }
        return true;
    }

    void xSocks5Client::Clean()
    {
        assert(_SocketHolder.IsValid());
        _SocketHolder.Delete();
        Reset(_IoContextPtr);
        Reset(_Username);
        Reset(_Password);
    }

    void xSocks5Client::ConnectionHandler(const asio::error_code& ErrorCode)
    {
        Todo();
    }

    bool xSocks5Proxy::Init(asio::io_context * IoContextPtr, const asio::ip::tcp::endpoint & BindAddress, iProxyAuth * AuthCallback)
    {
        assert(AuthCallback);
        try {
            _AcceptorHolder.New(*IoContextPtr);
            _AcceptorHolder->open(BindAddress.protocol());
            _AcceptorHolder->non_blocking(true);
            _AcceptorHolder->set_option(asio::ip::tcp::acceptor::reuse_address(true));
            _AcceptorHolder->bind(BindAddress);
            _AcceptorHolder->listen();
        }
        catch (...)
        {
            if (_AcceptorHolder.IsValid()) {
                _AcceptorHolder.Delete();
            }
            Reset(_Address);
            Reset(_IoContextPtr);
            Reset(_AuthCallback);
            return false;
        }
        return true;
    }

    void xSocks5Proxy::Clean()
    {
        assert(_AcceptorHolder.IsValid());
        _AcceptorHolder.Delete();
        Reset(_Address);
        Reset(_IoContextPtr);
        Reset(_AuthCallback);
    }

}
