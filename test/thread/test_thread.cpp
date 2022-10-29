#include <xel/Common.hpp>
#include <xel/Util/Thread.hpp>
#include <xel/Util/Chrono.hpp>
#include <iostream>

using namespace xel;
using namespace std;

int main(int, char **)
{
    xEvent ev;

    auto t0 = thread{[&]{
        ev.Wait([]{ cout << "Wait_0(" << GetTimestampMS() << ")" << endl; });
    }};

    auto t1 = thread{[&]{
        ev.Wait(
            []{ cout << "Wait_1_Pre(" << GetTimestampMS() << ")" << endl; },
            []{ cout << "Wait_1_Post(" << GetTimestampMS() << ")" << endl;}
        );
    }};

    auto t2 = thread{[&]{
        if (!ev.WaitFor(
            1s,
            []{ cout << "WaitFor_2_Pre(" << GetTimestampMS() << ")" << endl; },
            []{ cout << "WaittFor_2_Post(" << GetTimestampMS() << ")" << endl;}
        )) {
            cout << "WaitFor_2_Timeout" << endl;
        }
    }};

    this_thread::sleep_for(1s);
    auto tn = thread{[&]{
        ev.NotifyAll([]{
            this_thread::sleep_for(1s);
        });
    }};

    this_thread::sleep_for(1s);
    cout << "end of main thread" << endl;

    t0.join();
    t1.join();
    t2.join();
    tn.join();
    return 0;
}
