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
auto DnsClient = xLocalDnsClient();

int main(int argc, char *argv[])
{
    auto DnsServerAddress = xNetAddress::Parse("192.168.123.1:53");
    X_DEBUG_PRINTF("Using dns server: %s\n", DnsServerAddress.ToString().c_str());

    auto ServiceGuard = xResourceGuard(DnsService, DnsServerAddress);
    auto ClientGuard = xResourceGuard(DnsClient, &DnsService);

    auto Query = std::make_unique<xLocalDnsServer::xRequest>();
    Query->Hostname = "www.baidu.com";
    DnsService.DoSendDnsQuery(Query.get());

    auto Query2 = std::make_unique<xLocalDnsServer::xRequest>();
    Query2->Hostname = "www.qq.com";
    DnsService.DoSendDnsQuery(Query2.get());

    while(true) {
        DnsService.IoLoop();
    }

    assert(ServiceGuard);
    assert(ClientGuard);

    return 0;
}

