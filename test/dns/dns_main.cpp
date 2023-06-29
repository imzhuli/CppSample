#include <xel/Common.hpp>
#include <xel/Util/Command.hpp>
#include <xel_ext/IO/LocalDns.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

using namespace xel;
using namespace std;
using namespace std::literals;

auto DnsService = xLocalDnsServer();
static auto DnsServerAddress = xNetAddress::Parse("47.252.112.245:13221");

int main(int argc, char *argv[])
{
    auto Cmd = xCommandLine(argc, argv, {
        { 'd', "dns_server", "dns_server", true },
    });

    // make request first , so it's recycled last
    auto Query1 = std::make_unique<xLocalDnsServer::xRequest>();
    auto Query2 = std::make_unique<xLocalDnsServer::xRequest>();
    auto Query3 = std::make_unique<xLocalDnsServer::xRequest>();
    auto Query4 = std::make_unique<xLocalDnsServer::xRequest>();
    auto Query5 = std::make_unique<xLocalDnsServer::xRequest>();
    auto List = xList<xLocalDnsServer::xRequest>();

    auto OptDnsServerAddress = Cmd["dns_server"];
    if (OptDnsServerAddress()) {
        DnsServerAddress = xNetAddress::Parse(*OptDnsServerAddress);
        if (!DnsServerAddress) {
            X_DEBUG_PRINTF("Invalid dns server setting: %s\n", OptDnsServerAddress->c_str());
            return -1;
        }
        if (!DnsServerAddress.Port) {
            DnsServerAddress.Port = 53;
        }
    }

    X_DEBUG_PRINTF("Using dns server: %s\n", DnsServerAddress.ToString().c_str());

    auto ServiceGuard = xResourceGuard(DnsService, DnsServerAddress);
    assert(ServiceGuard);

    Query1->Hostname = "www.baidu.com";
    DnsService.PostQuery(Query1.get());

    std::this_thread::sleep_for(1s);

    Query2->Hostname = "www.gamesquad.top";
    DnsService.PostQuery(Query2.get());

    std::this_thread::sleep_for(1s);

    Query3->Hostname = "acp.zhuli.cool";
    DnsService.PostQuery(Query3.get());

    std::this_thread::sleep_for(2s);

    Query4->Hostname = "ue.zhuli.cool";
    DnsService.PostQuery(Query4.get());

    DnsService.Pick(List);
    DnsService.CancelAll();

    Query5->Hostname = "www.zhuli.cool";
    DnsService.PostQuery(Query5.get());

    for (auto & Result : List) {
        cout << "Result: " << Result.Hostname << " addr1=" << Result.Address1.IpToString() << " addr2=" << Result.Address2.IpToString() << endl;
        List.Remove(Result);
    }

    std::this_thread::sleep_for(2s);
    DnsService.Pick(List);
    for (auto & Result : List) {
        cout << "Cancelled Result: " << Result.Hostname << " addr1=" << Result.Address1.IpToString() << " addr2=" << Result.Address2.IpToString() << endl;
        List.Remove(Result);
    }

    return 0;
}
