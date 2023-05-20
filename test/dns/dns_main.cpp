#include <xel/Common.hpp>
#include <xel_ext/IO/Dns.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace xel;
using namespace std;
using namespace std::literals;

auto IoContext = xIoContext();
auto UserEventTriggerPtr = static_cast<xIoContext::iUserEventTrigger*>(nullptr);

void Trigger()
{
    UserEventTriggerPtr->Trigger();
}

int main(int argc, char *argv[])
{
    auto IoContextGuard = xResourceGuard(IoContext);
    UserEventTriggerPtr = IoContext.GetUserEventTrigger();

    auto Thread = std::thread([]{
        std::this_thread::sleep_for(1s);
        Trigger();
        std::this_thread::sleep_for(1s);
        Trigger();
    });

    IoContext.LoopOnce(3'000);
    cout << "UE1" << endl;
    IoContext.LoopOnce(3'000);
    cout << "UE2" << endl;

    Thread.join();
    return 0;
}

