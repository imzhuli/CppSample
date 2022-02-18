#include <zec_ext/IO/Socks5Proxy.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xSocks5Client::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, const std::string & Username, const std::string & Password)
    {
        assert(IoContextPtr);
        xNetAddress ProxyAddress = xNetAddress::Make(Ip);
        if (!ProxyAddress || !Port) {
            return false;
        }

        assert(!_IoContextPtr);
        _IoContextPtr = IoContextPtr;
        _ProxyAddress = ProxyAddress;
        _ProxyPort = Port;
        _Username = Username;
        _Password = Password;

        NativeTcpSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        return true;
    }

    void xSocks5Client::Clean()
    {
        NativeTcpSocketHolderRef(Native()).Destroy();
        Reset(_IoContextPtr);
        Reset(_ProxyAddress);
        Reset(_ProxyPort);
        Reset(_Username);
        Reset(_Password);
    }



}
