#pragma once

#include <xel/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./NetBase.hpp"
#include "./Packet.hpp"

X_NS
{
    class xUdpSocketContext;
    class xUdpChannel;

    class xUdpChannel final
    : xNonCopyable
    {
    public:
        struct iListener {
            virtual void OnError(xUdpChannel * ChannelPtr, const char * Message) {}
            virtual void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) = 0;
        };

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr);
        X_API_MEMBER void Clean();

        X_API_MEMBER void ResizeSendBuffer(size_t Size);
        X_API_MEMBER void ResizeReceiveBuffer(size_t Size);

        X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress);

    private:
        xUdpSocketContext *  _SocketPtr = nullptr;
    };

}
