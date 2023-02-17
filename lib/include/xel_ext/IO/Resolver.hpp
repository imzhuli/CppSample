#pragma once

#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/List.hpp>
#include <xel_ext/Util/TimeoutList.hpp>
#include <string>
#include <vector>
#include <map>
#include "./IoContext.hpp"
#include "./NetBase.hpp"

X_NS
{

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
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, iListener * ObserverPtr);
        X_API_MEMBER void Clean();
        X_API_MEMBER bool Request(const std::string & Hostname, const xVariable RequestContext = {});
        X_API_MEMBER void ClearTimeoutRequest();
        X_API_MEMBER void ClearTimeoutCacheNode();

    private:
        iListener *                                 _ListenerPtr = nullptr;
        uint64_t                                    _RequestTimeout = 3'000;
        uint64_t                                    _CacheTimeout = 600'000;
        xTimeoutList<>                              _RequestTimeoutList;
        xTimeoutList<>                              _CacheTimeoutList;
        std::map<std::string, xResolveNode>         _ResolveMap;
        xDummy<16>                                  _Native;
    };

}
