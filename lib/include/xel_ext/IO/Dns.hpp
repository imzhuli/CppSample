#include "./_.hpp"
#include "./NetAddress.hpp"
#include "./IoContext.hpp"
#include "./UdpChannel.hpp"
#include <vector>
#include <string>

X_NS
{

    class xDnsService
    {
    public:
        class iListener
        {
        public:
            void OnDnsResolveDone(const std::string & Hostname, const std::vector<xNetAddress> & AddressList);
            void OnDnsResolveFailed(const std::string & Hostname);
            void OnDnsResolveTimeout(const std::string & Timeout);
        };
        static constexpr const size_t DefaultCacheTimeoutMS = 10 * 60'000;

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const std::vector<xNetAddress> & ServerList, iListener * ListenerPtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER bool Query(const std::string & Hostname);
        X_API_MEMBER void Peek();

        X_API_MEMBER void SetMaxCacheSize();
        X_API_MEMBER void SetMaxQueryCount();

    private:
        size_t   MaxQueryCount        = static_cast<size_t>(-1);
        size_t   MaxCacheSize         = static_cast<size_t>(-1);
        uint64_t DnsCacheTimeoutMS    = DefaultCacheTimeoutMS;
    };

}
