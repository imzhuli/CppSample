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
            xNetAddress Ipv4;
            xNetAddress Ipv6;
        };

        static constexpr const size_t MaxQueryCount = 256;
        static constexpr const size_t DefaultCacheTimeoutMS = 10 * 60'000;

    public:
        X_API_MEMBER bool Init(const xNetAddress & Server);
        X_API_MEMBER void Clean();

    private: // for dns client:
        void PostQuery(xRequest * RequestPtr);

    private:
    public: // debug
        X_API_MEMBER void OnError(xUdpChannel * ChannelPtr) override;
        X_API_MEMBER void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
        X_API_MEMBER bool DoSendDnsQuery(xRequest * RequestPtr);
        X_API_MEMBER void DoPushResolvResult(uint16_t Index, const char * HostnameBuffer, const xNetAddress * ResolvedList, size_t ResolvedCounter);
        X_API_MEMBER void ReleaseQuery(uint16_t Index);
        X_INLINE     void ReleaseQuery(xRequest * RequestPtr) { ReleaseQuery((uint16_t)RequestPtr->Ident); }
        X_INLINE     void IoLoop() { IoContext.LoopOnce(1000); }

    private:
        xIoContext        IoContext            = {};
        xUdpChannel       UdpChannel           = {};
        xNetAddress       DnsServerAddress     = {};
        xSpinlock         SpinLock             = {};
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

    private:
        void RecycleRequest();

    private:
        xLocalDnsServer * LocalDnsServerPtr X_DEBUG_INIT(nullptr);
    };

}
