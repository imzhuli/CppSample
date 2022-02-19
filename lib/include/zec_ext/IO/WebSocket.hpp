#pragma once
#include <zec/Common.hpp>
#include <string>
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
            virtual void OnConnected(xWebSocketClient * WebSocketClientPtr) { }
            virtual void OnHandshakeDone(xWebSocketClient * WebSocketClientPtr) { }
            virtual bool OnMessage(xWebSocketClient * WebSocketClientPtr, const void * DataPtr, size_t DataSize) { return false; }
            virtual void OnPeerClose(xWebSocketClient * WebSocketClientPtr)  {}
            virtual void OnError(xWebSocketClient * WebSocketClientPtr) {}
        };

        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }
        ZEC_API_MEMBER void DoHandshake();
        ZEC_API_MEMBER void DoClose();
        ZEC_API_MEMBER void ErrorClose() { if (_State >= eClosing) { return; } _State = eClosing; _ListenerPtr->OnError(this); DoClose(); }

    private:
        xIoContext *     _IoContextPtr = nullptr;
        iListener *      _ListenerPtr = nullptr;
        void *           _FrameBufferPtr = nullptr;
        std::string      _Hostname;
        std::string      _Target;

        enum : uint8_t {
            eUnspecified, eInited, eConnected, eClosing, eClosed
        } _State = eUnspecified;

        alignas(max_align_t) ubyte    _Dummy[24];
        friend class __detail__::IOUtil;
    };

}
