#include <zec_ext/Net/Socks5Proxy.hpp>

ZEC_NS
{

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
            if (!_AcceptorHolder.IsValid()) {
                _AcceptorHolder.Delete();
            }
            return false;
        }
        return true;
    }

    void xSocks5Proxy::Clean()
    {
        assert(_AcceptorHolder.IsValid());
        _AcceptorHolder.Delete();
    }

}
