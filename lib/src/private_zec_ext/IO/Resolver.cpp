#include "./_Local.hpp"
#include <zec_ext/IO/Resolver.hpp>
#include <memory>

ZEC_NS
{

    namespace {
        using xNativeTcpResolver = tcp::resolver;
        struct xResolverContext
        {
            xNativeTcpResolver           Resolver;
            xTcpResolver::iListener *    ListenerPtr;
        };
        using xResolverHandle = std::shared_ptr<xResolverContext>;
    }

    #define DEF_RESOLVER_HANDLE() auto & Handle = _Native.As<xResolverHandle>()

    bool xTcpResolver::Init(xIoContext * IoContextPtr, iListener * ObserverPtr)
    {
        assert(IoContextPtr);
        assert(ObserverPtr);

        assert(!_ListenerPtr);
        assert(_RequestTimeoutList.IsEmpty());
        assert(_CacheTimeoutList.IsEmpty());
        assert(_ResolveMap.empty());

        _ListenerPtr = ObserverPtr;
        _Native.CreateValueAs<xResolverHandle>(new xResolverContext{ tcp::resolver{xIoCaster{}(*IoContextPtr)}, _ListenerPtr });

        return true;
    }

    void xTcpResolver::Clean()
    {
        _RequestTimeoutList.Finish([this](xTimeoutNode& TimeoutNode, xVariable){
                auto & Node = (xResolveNode&)TimeoutNode;
                for (const auto & Context : Node.RequestContexts) {
                    _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                }
            });
        _CacheTimeoutList.Finish([](xTimeoutNode&, xVariable){});
        _ResolveMap.clear();

        DEF_RESOLVER_HANDLE();
        Handle->ListenerPtr = nullptr;
        _Native.DestroyAs<xResolverHandle>();
        _ListenerPtr = nullptr;
    }

    bool xTcpResolver::Request(const std::string & Hostname, const xVariable RequestContext)
    {
        auto & Node = _ResolveMap[Hostname];
        if (Node.Address.Type == xNetAddress::eUnknown) {
            Node.RequestContexts.push_back(RequestContext);
            if (Node.GetTimestampMS()) { // during request
                return true;
            }
            Node.Hostname = Hostname;
            _RequestTimeoutList.PushBack(Node);

            DEF_RESOLVER_HANDLE();
            Handle->Resolver.async_resolve(Hostname, ""sv, [this, Handle, Hostname] (xAsioError error, const xNativeTcpResolver::results_type & results) {
                auto ListenerPtr = Handle->ListenerPtr;
                if (ListenerPtr == nullptr) { // closed resolver, and reference to *this is invalid
                    return;
                }

                auto Iter = _ResolveMap.find(Hostname);
                if (Iter == _ResolveMap.end()) { // request timeout
                    return;
                }
                auto & Node = Iter->second;

                if(error) {
                    /* Node.Address.Type = xNetAddress::eUnknown; */
                    for (const auto & Context : Node.RequestContexts) {
                        ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                    }
                    _ResolveMap.erase(Iter);
                    return;
                }

                for(const auto & record : results) {
                    auto endpoint = record.endpoint();
                    auto address = endpoint.address();
                    if (address.is_v4()) {
                        Node.Address.Type = xNetAddress::eIpv4;
                        auto AddrBytes = address.to_v4().to_bytes();
                        memcpy(Node.Address.Ipv4, AddrBytes.data(), sizeof(Node.Address.Ipv4));
                    } else if (address.is_v6()) {
                        Node.Address.Type = xNetAddress::eIpv6;
                        auto AddrBytes = address.to_v6().to_bytes();
                        memcpy(Node.Address.Ipv6, AddrBytes.data(), sizeof(Node.Address.Ipv6));
                    } else {
                        /* Node.Address.Type == xNetAddress::eUnknown; */
                    }
                    break;
                }
                for (const auto & Context : Node.RequestContexts) {
                    ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                }
                _CacheTimeoutList.PushBack(Node);
                Node.RequestContexts.clear();
            });
            return true;
        }

        _ListenerPtr->OnResolve(Node.Hostname, Node.Address, RequestContext, true);
        return true;
    }

    void xTcpResolver::ClearTimeoutRequest()
    {
        _RequestTimeoutList.PopTimeoutNodes(_RequestTimeout, [this](xTimeoutNode& TimeoutNode, xVariable){
            auto & Node = (xResolveNode&)TimeoutNode;
            for (const auto & Context : Node.RequestContexts) {
                _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
            }
            _ResolveMap.erase(Node.Hostname);
        });
    }

    void xTcpResolver::ClearTimeoutCacheNode()
    {
        _CacheTimeoutList.PopTimeoutNodes(_CacheTimeout, [this](xTimeoutNode& TimeoutNode, xVariable){
            auto & Node = (xResolveNode&)TimeoutNode;
            for (const auto & Context : Node.RequestContexts) {
                _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
            }
            _ResolveMap.erase(Node.Hostname);
        });
    }

}
