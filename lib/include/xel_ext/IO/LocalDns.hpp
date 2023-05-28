#include "./_.hpp"
#include "./NetAddress.hpp"
#include "./IoContext.hpp"
#include "./UdpChannel.hpp"
#include <xel/Util/Thread.hpp>
#include <xel/Util/IndexedStorage.hpp>
#include <vector>
#include <string>
#include <thread>

X_NS
{

    class xLocalDnsServer;

    class xLocalDnsServer final
    : public xUdpChannel::iListener
    , xNonCopyable
    {
    public:
        static constexpr const size_t MaxQueryCount = 256;

        class xRequest
        : public xListNode
        {
        public:
            xIndexId    Ident = {};
            uint64_t    TimestampMS = 0;

            std::string Hostname;
            xNetAddress Address1;
            xNetAddress Address2;
        };

        using NotifyCallback = void (xVariable);

    public:
        X_API_MEMBER bool Init(const xNetAddress & Server, NotifyCallback * NotifyCallbackPtr = nullptr, xVariable NotifyVariable = {});
        X_API_MEMBER void Clean();

    public:
        X_API_MEMBER void SetDnsServer(const xNetAddress & NewAddress);
        X_API_MEMBER void PostQuery(xRequest * RequestPtr);
        X_API_MEMBER void Pick(xList<xRequest> & Receiver);

    private:
        X_API_MEMBER void OnError(xUdpChannel * ChannelPtr) override;
        X_API_MEMBER bool OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;

        X_PRIVATE_MEMBER bool DoSendDnsQuery(xRequest * RequestPtr);
        X_PRIVATE_MEMBER void DoPushResolvResult(uint16_t Index, const char * HostnameBuffer, const xNetAddress * ResolvedList, size_t ResolvedCounter);
        X_PRIVATE_MEMBER void ReleaseQuery(uint16_t Index);
        X_PRIVATE_MEMBER void ReleaseQuery(xRequest * RequestPtr) { ReleaseQuery((uint16_t)RequestPtr->Ident); }
        X_PRIVATE_MEMBER void Loop();

    private:
        xIoContext        IoContext                   = {};
        xUdpChannel       UdpChannel                  = {};
        xNetAddress       DnsServerAddress            = {};
        xSpinlock         Spinlock                    = {};
        xList<xRequest>   RequestList                 = {};
        xList<xRequest>   RequestTimeoutList          = {};
        xList<xRequest>   InternalRequestResultList   = {};
        xList<xRequest>   ExchangeRequestResultList   = {};

        xNetAddress       NewDnsServerAddress         = {};

        xIndexedStorage<xRequest*>   IdPool;
        std::vector<bool>            IdMarks;

        std::thread        ServiceThread;
        std::atomic_bool   StopFlag;

        NotifyCallback *   NotifyCallbackPtr;
        xVariable          NotifyVariable;
    };

}
