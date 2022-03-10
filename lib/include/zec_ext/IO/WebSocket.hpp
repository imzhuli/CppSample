#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <string>
#include <memory>
#include "./IoContext.hpp"

ZEC_NS
{

    class xWebSocketSession
    : xNonCopyable
    {
    public:
        struct iListener
        {
            virtual void OnHandshakeDone(xWebSocketSession * WebSocketClientPtr) {}
            virtual void OnMessage(xWebSocketSession * WebSocketClientPtr, bool Binary, const void * DataPtr, size_t DataSize) {}
            virtual void OnError(xWebSocketSession * WebSocketClientPtr) {}
        };

    public:
        ZEC_PRIVATE_MEMBER xWebSocketSession();
        ZEC_PRIVATE_MEMBER ~xWebSocketSession();

        ZEC_PRIVATE_MEMBER bool Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Origin, const std::string &Target, iListener * ListenerPtr);
        ZEC_PRIVATE_MEMBER void Clean();

        ZEC_PRIVATE_MEMBER bool PostTextData(const std::string_view & Data);
        ZEC_PRIVATE_MEMBER bool PostBinaryData(const void * DataPtr, size_t DataSize);

    protected:
        struct xMessageBuffer : xListNode {
            std::string Data;
            bool        Binary = false;
        };
        using xMessageBufferList = xList<xMessageBuffer>;

        ZEC_PRIVATE_MEMBER bool OnError(const void * CallbackObjectPtr); // return value : processed
        ZEC_PRIVATE_MEMBER void OnConnected(const void * CallbackObjectPtr);
        ZEC_PRIVATE_MEMBER void OnHandshakeDone(const void * CallbackObjectPtr);
        ZEC_PRIVATE_MEMBER bool DoPostMessage(xMessageBuffer * MessagePtr);
        ZEC_PRIVATE_MEMBER void DoRead();
        ZEC_PRIVATE_MEMBER void DoFlush();

    private:
        xDummy<16>   _Native; // active websocket object
        iListener*   _Listener;
        std::string  _Origin;
        std::string  _Path;
        bool         _Connected = false;
        bool         _Error = false;

        xDummy<16>           _ReadBuffer; // active websocket object
        xMessageBufferList   _MessageBufferList;
    };

}
