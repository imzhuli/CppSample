#include <xel/Common.hpp>
#include <xel_ext/IO/LocalDns.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

using namespace xel;
using namespace std;
using namespace std::literals;

auto DnsService = xLocalDnsServer();

int main(int argc, char *argv[])
{
    // make request first , so it's recycled last
    auto Query1 = std::make_unique<xLocalDnsServer::xRequest>();
    auto Query2 = std::make_unique<xLocalDnsServer::xRequest>();

    auto DnsServerAddress = xNetAddress::Parse("192.168.123.1:53");
    X_DEBUG_PRINTF("Using dns server: %s\n", DnsServerAddress.ToString().c_str());

    auto ServiceGuard = xResourceGuard(DnsService, DnsServerAddress);
    assert(ServiceGuard);

    Query1->Hostname = "www.baidu.com";
    DnsService.PostQuery(Query1.get());
    Query2->Hostname = "www.qq.com";
    DnsService.PostQuery(Query2.get());

    std::this_thread::sleep_for(1s);
    auto List = xList<xLocalDnsServer::xRequest>();
    DnsService.Pick(List);

    for (auto & Result : List) {
        cout << "Result: " << Result.Hostname << " addr1=" << Result.Address1.IpToString() << " addr2=" << Result.Address2.IpToString() << endl;
        List.Remove(Result);
    }

    return 0;
}

