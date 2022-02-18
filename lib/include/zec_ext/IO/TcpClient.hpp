#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./PacketData.hpp"

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
            virtual bool OnDataIn()     { return false; }
            virtual bool OnDataOut()    { return false; }
            virtual void OnPeerClose()  { }
        };

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

        ZEC_API_MEMBER size_t Read(void * BufferPtr, size_t BufferSize);
        ZEC_API_MEMBER void   Write(const void * DataPtr, size_t DataSize);

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        xIoContext *                  _IoContextPtr {};
        xNetAddress                   _ServerAddress {};
        uint16_t                      _ServerPort {};
        iListener *                   _ListenerPtr = nullptr;
        xPacketBufferQueue            _PacketBufferQueue;

        alignas(max_align_t) ubyte    _Dummy[64];
        friend class __detail__::IOUtil;
    };

}
