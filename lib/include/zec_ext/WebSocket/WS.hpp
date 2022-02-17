#pragma once
#include <zec/Common.hpp>
#include <string>
#include <list>

ZEC_NS
{
    namespace __detail__ {
        class xWebSocketWrapper;
    };

    struct xWebSocketSession;
    class xWebSocketContext;
    class xWebSocketClient;

    class xWebSocketContext final
    {
    public:
        bool Init();
        void LoopOnce();
        void Clean();

    private:
        xVariable _WSContextPtr = { .Ptr = nullptr };

    private:
        friend class __detail__::xWebSocketWrapper;
        friend class xWebSocketClient;
    };

    class xWebSocketClient
    {
    public:
        struct xConfig {
            std::string Hostname;
            uint16_t    Port;
            std::string Origin;
            std::string Path;
        };
        ZEC_INLINE bool IsConnected() const { return _WSSocketSessionPtr; }
        ZEC_API_MEMBER bool Init(xWebSocketContext * WSContextPtr, const xConfig & Config);
        ZEC_API_MEMBER void Post(const void * DataPtr, size_t Size);
        ZEC_API_MEMBER void Clean();

    protected:
        virtual bool OnWSDataIn(const void * DataPtr, size_t DataSize) = 0;
        virtual void OnWSDisconnected() = 0;
        virtual void OnWSDataOutReady() {};

    private:
        xWebSocketSession *    _WSSocketSessionPtr = nullptr;
        std::list<std::string> _RelayMessages;

    private:
        friend class __detail__::xWebSocketWrapper;
    };

}
