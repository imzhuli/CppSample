#include <xel_ext/IO/LocalDns.hpp>
#include <xel/String.hpp>

X_NS
{

    bool xLocalDnsServer::Init(const xNetAddress & ServerAddress)
    {
        assert(ServerAddress);
        if (!IoContext.Init()) {
            return false;
        }
        auto IoContextGuard = MakeCleaner(IoContext);

        auto BindAddress = ServerAddress.IsV4() ? xNetAddress::Make4() : xNetAddress::Make6();
        if (UdpChannel.Init(&IoContext, BindAddress, this)) {
        // if (UdpChannel.Init(&IoContext, ServerAddress.GetAddressFamily(), this)) {
            return false;
        }
        auto UdpChannelGuard = MakeCleaner(UdpChannel);

        UdpChannelGuard.Dismiss();
        IoContextGuard.Dismiss();
        return true;
    }

    void xLocalDnsServer::Clean()
    {
        UdpChannel.Clean();
        IoContext.Clean();
    }

    void xLocalDnsServer::OnError(xUdpChannel * ChannelPtr)
    {
        Fatal("UdpChannelError");
    }

    void xLocalDnsServer::OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress)
    {
        auto Hex = xel::StrToHex(DataPtr, DataSize);
        X_DEBUG_PRINTF("UdpData: \n%s\n", Hex.c_str());
    }

    /******** Client ************/

    bool xLocalDnsClient::Init(xLocalDnsServer * DnsServicePtr)
    {
        return true;
    }

    void xLocalDnsClient::Clean()
    {

    }

}
