#include <xel/Common.hpp>
#include <xel_ext/IO/LocalDns.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace xel;
using namespace std;
using namespace std::literals;

auto DnsService = xLocalDnsServer();
auto DnsClient = xLocalDnsClient();

int main(int argc, char *argv[])
{
    auto DnsServerAddress = xNetAddress::Parse("113.96.236.11:53");
    X_DEBUG_PRINTF("Using dns server: %s\n", DnsServerAddress.ToString().c_str());

    auto ServiceGuard = xResourceGuard(DnsService, DnsServerAddress);
    auto ClientGuard = xResourceGuard(DnsClient, &DnsService);

    assert(ServiceGuard);
    assert(ClientGuard);

    return 0;
}

