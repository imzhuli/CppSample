#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xTcpClient
    : xNonCopyable
    {
    public:
        struct iListener
        {
            virtual bool OnConnected()  { return false; }
            virtual bool OnData()       { return false; }
            virtual void OnPeerClose()  { }
        };

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        xIoContext *                  _IoContextPtr {};
        xNetAddress                   _ServerAddress {};
        uint16_t                      _ServerPort {};
        iListener *                   _ListenerPtr = nullptr;

        alignas(max_align_t) ubyte    _Dummy[64];
        friend class __detail__::IOUtil;
    };

}
