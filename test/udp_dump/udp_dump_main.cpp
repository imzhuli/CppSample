#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel_ext/IO/UdpChannel.hpp>
#include <iostream>

using namespace std;
using namespace xel;

auto BindAddress = xNetAddress::Parse("0.0.0.0:53");
struct xUdpListener
: xUdpChannel::iListener
{
    void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress)
    {
        cout << "Dump udp, size=" << DataSize << ", SourceAddress=" << RemoteAddress.ToString() << endl;
        cout << HexShow(DataPtr, DataSize) << endl;
    };

} Dumper;

int main(int, char **)
{
    auto IoContext = xIoContext();
    auto ICG = xResourceGuard(IoContext);
    assert(ICG);

    auto UdpServer = xUdpChannel();
    auto USG = xResourceGuard(UdpServer, &IoContext, BindAddress, &Dumper);

    while(true) {
        IoContext.LoopOnce();
    }

    return 0;
}
