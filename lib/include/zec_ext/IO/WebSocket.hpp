#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <string>
#include <memory>
#include "./IoContext.hpp"
#include "./NetBase.hpp"

ZEC_NS
{

    class xWebSocketSession
    : xNonCopyable
    {
    public:
        struct iListener
        {
            virtual void OnWSHandshakeDone(xWebSocketSession * WebSocketClientPtr) {}
            virtual void OnWSMessage(xWebSocketSession * WebSocketClientPtr, bool Binary, void * DataPtr, size_t DataSize) {}
            virtual void OnWSClose(xWebSocketSession * WebSocketClientPtr) {}
        };

    public:
        ZEC_API_MEMBER xWebSocketSession();
        ZEC_API_MEMBER ~xWebSocketSession();

        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * IpStr, uint16_t Port, const std::string & Origin, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, const std::string & Origin, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();
        ZEC_INLINE     bool IsActive() const { return _Active; }

        ZEC_API_MEMBER bool PostTextData(const std::string_view & Data);
        ZEC_API_MEMBER bool PostBinaryData(const void * DataPtr, size_t DataSize);

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
        bool         _Active = false;
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
