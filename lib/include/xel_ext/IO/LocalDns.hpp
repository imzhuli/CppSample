#include "./_.hpp"
#include "./NetAddress.hpp"
#include "./IoContext.hpp"
#include "./UdpChannel.hpp"
#include <xel/Util/Thread.hpp>
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
            std::string Hostname;
            xNetAddress Ipv4;
            xNetAddress Ipv6;
        };

        static constexpr const size_t DefaultCacheTimeoutMS = 10 * 60'000;

    public:
        X_API_MEMBER bool Init(const xNetAddress & Server);
        X_API_MEMBER void Clean();

    private: // for dns client:
        void Post(xRequest * RequestPtr);

    private:
        X_API_MEMBER void OnError(xUdpChannel * ChannelPtr) override;
        X_API_MEMBER void OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;

    private:
        xIoContext      IoContext            = {};
        xUdpChannel     UdpChannel           = {};
        xNetAddress     DnsServerAddress     = {};
        xSpinlock       SpinLock             = {};
        xList<xRequest> RequestList          = {};

        size_t        MaxQueryCount        = static_cast<size_t>(-1);
        size_t        MaxCacheSize         = static_cast<size_t>(-1);
        uint64_t      DnsCacheTimeoutMS    = DefaultCacheTimeoutMS;

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
        xLocalDnsServer * ServicePtr X_DEBUG_INIT(nullptr);
    };

}
