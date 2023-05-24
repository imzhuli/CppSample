#pragma once

#include "./_.hpp"
#include "./IoContext.hpp"
#include "./NetAddress.hpp"
#include "./Packet.hpp"

X_NS
{

    class xUdpChannel final
    : public iBufferedIoReactor
    , xAbstract
    {
    public:
        struct iListener {
            virtual void OnError(xUdpChannel * ChannelPtr) {}
            virtual void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) = 0;
        };

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, int AddressFamily, iListener * ListenerPtr);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress);

    private:
        X_API_MEMBER void OnIoEventInReady() override;
        X_API_MEMBER void OnIoEventError() override;

    #if defined(X_SYSTEM_WINDOWS)
        X_API_MEMBER eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) override;
        X_API_MEMBER void TryRecvData();
    #endif

    private:
        xIoContext * _IoContextPtr;
        xSocket      _Socket X_DEBUG_INIT(InvalidSocket);
        iListener *  _ListenerPtr X_DEBUG_INIT(nullptr);
    };

}
