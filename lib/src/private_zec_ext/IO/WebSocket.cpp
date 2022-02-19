#include <zec_ext/IO/WebSocket.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xWebSocketClient::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, size_t FrameBufferSize, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(FrameBufferSize);

        assert(!IoContextPtr);
        assert(!ListenerPtr);

        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _FrameBufferPtr = new ubyte [FrameBufferSize];
        _FrameBufferSize = FrameBufferSize;
        _FrameDataSize = 0;
        _State = eInited;
        return true;
    }

    void xWebSocketClient::Clean()
    {
        delete (Steal(_FrameBufferPtr));
        Reset(_FrameDataSize);
        Reset(_FrameBufferSize);
        Reset(_ListenerPtr);
        Reset(_IoContextPtr);
        _State = eUnspecified;
    }

}
