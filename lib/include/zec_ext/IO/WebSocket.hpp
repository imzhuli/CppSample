#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <string>
#include <memory>
#include "./IoContext.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

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
        xWebSocketSession();
        ~xWebSocketSession();

        bool Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Origin, const std::string &Target, iListener * ListenerPtr);
        void Clean();

        bool PostTextData(const std::string_view & Data);
        bool PostBinaryData(const void * DataPtr, size_t DataSize);

    protected:
        struct xMessageBuffer : xListNode {
            std::string Data;
            bool        Binary = false;
        };
        using xMessageBufferList = xList<xMessageBuffer>;

        bool OnError(const void * CallbackObjectPtr); // return value : processed
        void OnConnected(const void * CallbackObjectPtr);
        void OnHandshakeDone(const void * CallbackObjectPtr);
        bool DoPostMessage(xMessageBuffer * MessagePtr);
        void DoRead();
        void DoFlush();

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
