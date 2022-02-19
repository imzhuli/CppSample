#pragma once
#include <zec/Common.hpp>

#include "./IoContext.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xWebSocketClient
    : xNonCopyable
    {
    public:
        struct iListener
        {
            virtual bool OnConnected(xWebSocketClient * WebSocketClientPtr)  { return true; }
            virtual bool OnReceiveData(xWebSocketClient * WebSocketClientPtr, const void * DataPtr, size_t DataSize) { return false; }
            virtual void OnPeerClose(xWebSocketClient * WebSocketClientPtr)  {}
            virtual void OnError(xWebSocketClient * WebSocketClientPtr) {}
        };

        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, size_t FrameBufferSize, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, size_t FrameBufferSize, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

    private:
        xIoContext *  _IoContextPtr = nullptr;
        iListener *   _ListenerPtr = nullptr;
        ubyte *       _FrameBufferPtr = nullptr;
        size_t        _FrameBufferSize = 0;
        size_t        _FrameDataSize = 0;

        alignas(max_align_t) ubyte    _Dummy[64];
        friend class __detail__::IOUtil;
    };

}
