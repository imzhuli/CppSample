#pragma once

#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec_ext/Utility/TimeoutList.hpp>
#include <string_view>
#include <string>
#include <vector>
#include <map>

#include "./IoContext.hpp"

ZEC_NS
{
    namespace __detail__ {
        class IOUtil;
    }

    class xTcpResolver
    : xNonCopyable
    {
    public:
        struct iListener {
            virtual void OnResolve(const std::string & Hostname, const xNetAddress & Address, const xVariable & RequestContext, bool DirectCall) = 0;
        };

    private:
        struct xResolveNode
        : xTimeoutNode
        {
            std::string              Hostname;
            xNetAddress              Address;
            std::vector<xVariable>   RequestContexts;
        };

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ObserverPtr);
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER bool Request(const std::string & Hostname, const xVariable RequestContext = {});
        ZEC_API_MEMBER void ClearTimeoutRequest();
        ZEC_API_MEMBER void ClearTimeoutCacheNode();

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        xIoContext *                                _IoContextPtr = nullptr;
        iListener *                                 _ListenerPtr = nullptr;
        uint64_t                                    _RequestTimeout = 3'000;
        uint64_t                                    _CacheTimeout = 600'000;
        xTimeoutList                                _RequestTimeoutList;
        xTimeoutList                                _CacheTimeoutList;
        std::map<std::string, xResolveNode>         _ResolveMap;

        alignas(max_align_t) ubyte                  _Dummy[80];
        friend class __detail__::IOUtil;
    };

}
