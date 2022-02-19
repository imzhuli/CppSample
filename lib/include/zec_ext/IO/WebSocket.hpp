#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
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
            virtual void OnConnected(xWebSocketClient * WebSocketClientPtr) {}
            virtual void OnHandshakeDone(xWebSocketClient * WebSocketClientPtr) {}
            virtual void OnMessage(xWebSocketClient * WebSocketClientPtr, bool Binary, const void * DataPtr, size_t DataSize) {}
            virtual void OnError(xWebSocketClient * WebSocketClientPtr) {}
        };

        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

        ZEC_INLINE     bool PostData(const void * DataPtr, size_t DataSize, bool Binary = false) { return PostData({ (const char *)DataPtr, DataSize }, Binary); }
        ZEC_API_MEMBER bool PostData(const std::string_view & DataView, bool Binary = false);

    private:
        struct xMessageBuffer : xListNode
        {
            std::string Data;
            bool        Binary = false;
        };
        using xMessageBufferList = xList<xMessageBuffer>;

    protected:
        ZEC_API_MEMBER virtual xMessageBuffer * NewMessageBuffer() { return new (std::nothrow) xMessageBuffer(); }
        ZEC_API_MEMBER virtual void DeleteMessageBuffer(xMessageBuffer * BufferPtr) { delete BufferPtr; }

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }
        ZEC_API_MEMBER void DoHandshake();
        ZEC_API_MEMBER void DoRead();
        ZEC_API_MEMBER void DoFlush();

    private:
        xIoContext *           _IoContextPtr = nullptr;
        iListener *            _ListenerPtr = nullptr;

        std::string            _Hostname;
        std::string            _Target;

        void *                 _ReadBufferPtr = nullptr;
        xMessageBufferList       _MessageBufferList;

        enum : uint8_t {
            eUnspecified, eInited, eConnected, eShuttingDown
        } _State = eUnspecified;

        alignas(max_align_t) ubyte    _Dummy[16];
        friend class __detail__::IOUtil;
    };

}
