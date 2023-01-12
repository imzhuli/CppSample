#pragma once

#include "./IoBase.hpp"
#include "./IoContext.hpp"
#include "./NetAddress.hpp"
#include "./Packet.hpp"

X_NS
{

    class xUdpChannel final
    : public iIoReactor
    , xAbstract
    {
    public:
        struct iListener {
            virtual void OnError(xUdpChannel * ChannelPtr) {}
            virtual void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) = 0;
        };

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress);

    private:
        X_API_MEMBER void OnIoEventInReady() override;
        X_API_MEMBER void OnIoEventError() override;

    private:
        xSocket      _Socket X_DEBUG_INIT(InvalidSocket);
        iListener *  _ListenerPtr X_DEBUG_INIT(nullptr);
    };

}
