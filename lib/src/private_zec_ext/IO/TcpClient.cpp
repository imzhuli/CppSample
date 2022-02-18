#include <zec_ext/IO/TcpClient.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xTcpClient::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        xNetAddress ProxyAddress = xNetAddress::Make(Ip);
        if (!ProxyAddress || !Port) {
            return false;
        }

        assert(!_IoContextPtr);
        _IoContextPtr = IoContextPtr;
        _ServerAddress = ProxyAddress;
        _ServerPort = Port;
        _ListenerPtr = ListenerPtr;

        NativeTcpSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        return true;
    }

    void xTcpClient::Clean()
    {
        assert(_PacketBufferQueue.IsEmpty());
        NativeTcpSocketHolderRef(Native()).Destroy();
        Reset(_IoContextPtr);
        Reset(_ServerAddress);
        Reset(_ServerPort);
    }

    size_t xTcpClient::Read(void * BufferPtr, size_t BufferSize)
    {
        // TODO
        return 0;
    }

    void xTcpClient::Write(const void * DataPtr, size_t DataSize)
    {
        // TODO

    }

}
