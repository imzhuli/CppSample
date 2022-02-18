#include <zec_ext/IO/Socks5Proxy.hpp>

ZEC_NS
{

    bool xSocks5Client::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, const std::string & Username, const std::string & Password)
    {
        if (!_TcpClient.Init(IoContextPtr, Ip, Port, this)) {
            return false;
        }

        _Username = Username;
        _Password = Password;
        return true;
    }

    void xSocks5Client::Clean()
    {
        _TcpClient.Clean();
        Reset(_Username);
        Reset(_Password);
    }

}
