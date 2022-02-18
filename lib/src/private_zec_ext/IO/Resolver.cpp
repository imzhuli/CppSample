#include "./_Local.hpp"
#include <zec_ext/IO/Resolver.hpp>

ZEC_NS
{
    using namespace std::literals::string_view_literals;

    bool xResolver::Init(xIoContext * IoContextPtr, iListener * ObserverPtr)
    {
        assert(IoContextPtr);
        assert(ObserverPtr);

        assert(!_IoContextPtr);
        assert(!_ListenerPtr);
        assert(_RequestTimeoutList.IsEmpty());
        assert(_CacheTimeoutList.IsEmpty());
        assert(_ResolveMap.empty());

        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ObserverPtr;

        NativeResolverHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        return true;
    }

    void xResolver::Clean()
    {
        _RequestTimeoutList.Finish([this](xTimeoutNode& TimeoutNode, xVariable){
                auto & Node = (xResolveNode&)TimeoutNode;
                for (const auto & Context : Node.RequestContexts) {
                    _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                }
            });
        _CacheTimeoutList.Finish([](xTimeoutNode&, xVariable){});
        _ResolveMap.clear();
        _ListenerPtr = nullptr;

        NativeResolverHolderRef(Native()).Destroy();
        _IoContextPtr = nullptr;
    }

    bool xResolver::Request(const std::string & Hostname, const xVariable RequestContext)
    {
        auto & Node = _ResolveMap[Hostname];
        if (Node.Address.Type == xNetAddress::eUnknown) {
            Node.RequestContexts.push_back(RequestContext);
            if (Node.GetTimestampMS()) { // during request
                return true;
            }
            Node.Hostname = Hostname;
            _RequestTimeoutList.PushBack(Node);
            auto & NativeHolder = NativeResolverHolderRef(Native());
            NativeHolder->async_resolve(Hostname, ""sv, [this, Hostname] (error_code error, const xNativeResolver::results_type & results) {
                auto Iter = _ResolveMap.find(Hostname);
                if (Iter == _ResolveMap.end()) { // request timeout
                    return;
                }
                auto & Node = Iter->second;
                if(!error) {
                    for(const auto & record : results) {
                        auto endpoint = record.endpoint();
                        auto address = endpoint.address();
                        if (address.is_v4()) {
                            Node.Address.Type = xNetAddress::eIpv4;
                            auto AddrBytes = address.to_v4().to_bytes();
                            memcpy(Node.Address.Ipv4, AddrBytes.data(), AddrBytes.size());
                        } else if (address.is_v6()) {
                            Node.Address.Type = xNetAddress::eIpv6;
                            auto AddrBytes = address.to_v6().to_bytes();
                            memcpy(Node.Address.Ipv6, AddrBytes.data(), AddrBytes.size());
                        } else {
                            /* Node.Address.Type == xNetAddress::eUnknown; */
                        }
                        break;
                    }
                    for (const auto & Context : Node.RequestContexts) {
                        _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                    }
                    _CacheTimeoutList.PushBack(Node);
                } else {
                    /* Node.Address.Type = xNetAddress::eUnknown; */
                    for (const auto & Context : Node.RequestContexts) {
                        _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
                    }
                    _ResolveMap.erase(Iter);
                }
            });
            return true;
        }

        _ListenerPtr->OnResolve(Node.Hostname, Node.Address, RequestContext, true);
        return true;
    }

    void xResolver::ClearTimeoutRequest()
    {
        _RequestTimeoutList.PopTimeoutNodes(_RequestTimeout, [this](xTimeoutNode& TimeoutNode, xVariable){
            auto & Node = (xResolveNode&)TimeoutNode;
            for (const auto & Context : Node.RequestContexts) {
                _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
            }
            _ResolveMap.erase(Node.Hostname);
        });
    }

    void xResolver::ClearTimeoutCacheNode()
    {
        _RequestTimeoutList.PopTimeoutNodes(_RequestTimeout, [this](xTimeoutNode& TimeoutNode, xVariable){
            auto & Node = (xResolveNode&)TimeoutNode;
            for (const auto & Context : Node.RequestContexts) {
                _ListenerPtr->OnResolve(Node.Hostname, Node.Address, Context, false);
            }
            _ResolveMap.erase(Node.Hostname);
        });
    }

}
