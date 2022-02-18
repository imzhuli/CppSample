#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/Resolver.hpp>
#include <zec/Util/Chrono.hpp>
#include <iostream>

using namespace zec;
using namespace std;

class xResolverListener
: public xResolver::iListener
{
    void OnResolve(const std::string & Hostname, const xNetAddress & Address, const xVariable & RequestContext, bool DirectCall) override
    {
        cout << "Resolve result: " << Hostname << ", RequestId=" << RequestContext.I << ", Result=" << Address.ToString() << ", Cached=" << YN(DirectCall) << endl;
    }
};

void test0 ()
{
    xNetAddress Addr = {};
    Addr.Type = xNetAddress::eIpv4;
    Addr.Ipv4[0] = 192;
    Addr.Ipv4[1] = 168;
    Addr.Ipv4[2] = 123;
    Addr.Ipv4[3] = 24;
    cout << Addr.ToString() << endl;

    xIoContext          IoContext;
    xResolver           Resolver;
    xResolverListener   Listener;

    if (!IoContext.Init()) {
        throw "Failed to init io context";
    }
    if (!Resolver.Init(&IoContext, &Listener)) {
        throw "Failed to init resolver";
    }

    Resolver.Request("www.baidu.com", { .I = 1 });
    Resolver.Request("www.baidu.com", { .I = 2 });
    Resolver.Request("www.qq.com", { .I = 3 });
    Resolver.Request("www.google.somethingiswrong.com", { .I = 4 });
    Resolver.Request("www.verylong_maybe_unresolved_name.com", { .I = 5 });
    Resolver.Request("www.invalid_    _name.com", { .I = 6 });

    xTimer Timer;
    while(!Timer.TestAndTag(3s)) {
        IoContext.LoopOnce(100);
        // Resolver.ClearTimeoutRequest();
    }
    Resolver.ClearTimeoutCacheNode();
    Resolver.Request("www.baidu.com", { .I = 100 });

    Resolver.Clean();
    IoContext.Clean();
}


int main(int, char **)
{
    try {
        test0();
    }
    catch (const char * Reason) {
        cout << Reason << endl;
        return -1;
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}