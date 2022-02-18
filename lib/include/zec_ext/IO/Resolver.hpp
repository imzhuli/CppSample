#pragma once

#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec_ext/Utility/TimeoutList.hpp>
#include "./IoContext.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <map>

ZEC_NS
{
    namespace __detail__ {
        class IOUtil;
    }

    class xResolver
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
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ObserverPtr, size32_t MaxRequestHint = 0);
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER bool Request(const std::string & Hostname, const xVariable RequestContext = {});
        ZEC_API_MEMBER void ClearTimeoutRequest();
        ZEC_API_MEMBER void ClearTimeoutCacheNode();

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }

    private:
        xIoContext *                                _IoContextPtr = nullptr;
        iListener *                                 _ListenerPtr = nullptr;
        uint64_t                                    _RequestTimeout = 100'000;
        uint64_t                                    _CacheTimeout = 600'000;
        xTimeoutList                                _RequestTimeoutList;
        xTimeoutList                                _CacheTimeoutList;
        std::map<std::string, xResolveNode>         _ResolveMap;
        alignas(max_align_t) ubyte                  _Dummy[48];

        friend class __detail__::IOUtil;
    };

}
