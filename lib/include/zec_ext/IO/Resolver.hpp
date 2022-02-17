#pragma once

#include <zec/Common.hpp>
#include <zec/List.hpp>
#include "./IoContext.hpp"
#include <map>
#include <string>
#include <string_view>

ZEC_NS
{


    class xResolver
    : xNonCopyable
    {
    public:
        struct iListener {
            void OnResolve(const std::string & Hostname, const xNetAddress & Address, const xVariable & RequestContext, bool DirectCall);
        };

    private:
        struct xRequestNode : xListNode{};
        using xRequestList = xList<xRequestNode>;

        struct xLRUNode : xListNode {};
        using xLRUList = xList<xLRUNode>;

        struct xRequest : xRequestNode {
            xVariable RequestContext;
        };

        struct xCacheNode : xLRUNode {
            xNetAddress    Address;
            uint64_t       Timestamp;
            xRequestList   RequestList;
        };

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ObserverPtr, size32_t MaxRequestHint);
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER bool Request(const std::string & Hostname, const xVariable RequestContext);

    private:
        xIoContext *                        _IoContextPtr = nullptr;
        uint64_t                            _CacheTimeout = 600;
        xLRUList                            _CacheTimeoutList;
        std::map<std::string, xCacheNode>   _CacheMap;
    };

}
