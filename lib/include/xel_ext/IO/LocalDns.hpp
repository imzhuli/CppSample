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

    class xLocalDnsClient;
    class xLocalDnsServer;

    class xLocalDnsServer final
    : public xUdpChannel::iListener
    , xNonCopyable
    {
        friend class xLocalDnsClient;
    public:
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

        static constexpr const size_t MaxQueryCount = 256;

    public:
        X_API_MEMBER bool Init(const xNetAddress & Server);
        X_API_MEMBER void Clean();

    public:
        void PostQuery(xRequest * RequestPtr);
        void Pick(xList<xRequest> & Receiver);

    private:
        X_PRIVATE_MEMBER void OnError(xUdpChannel * ChannelPtr) override;
        X_PRIVATE_MEMBER void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
        X_PRIVATE_MEMBER bool DoSendDnsQuery(xRequest * RequestPtr);
        X_PRIVATE_MEMBER void DoPushResolvResult(uint16_t Index, const char * HostnameBuffer, const xNetAddress * ResolvedList, size_t ResolvedCounter);
        X_PRIVATE_MEMBER void ReleaseQuery(uint16_t Index);
        X_PRIVATE_MEMBER void ReleaseQuery(xRequest * RequestPtr) { ReleaseQuery((uint16_t)RequestPtr->Ident); }
        X_PRIVATE_MEMBER void Loop();

    private:
        xIoContext        IoContext            = {};
        xUdpChannel       UdpChannel           = {};
        xNetAddress       DnsServerAddress     = {};
        xSpinlock         Spinlock             = {};
        xList<xRequest>   RequestList          = {};
        xList<xRequest>   RequestTimeoutList   = {};
        xList<xRequest>   RequestResultList    = {};

        xIndexedStorage<xRequest*>   IdPool;
        std::vector<bool>            IdMarks;

        std::thread        ServiceThread;
        std::atomic_bool   StopFlag;
    };

    class xLocalDnsClient final
    : xNonCopyable
    {
        friend class xLocalDnsServer;
    public:
        X_API_MEMBER bool Init(xLocalDnsServer * DnsServicePtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER bool Query(const std::string & Hostname);
        X_API_MEMBER void Pick();

    private:
        void RecycleRequest();

    private:
        xLocalDnsServer * LocalDnsServerPtr X_DEBUG_INIT(nullptr);
    };

}
