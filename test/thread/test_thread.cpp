#include <zec/Common.hpp>
#include <zec/Util/Thread.hpp>
#include <zec/Util/Chrono.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int main(int, char **)
{
    xEvent ev;

    auto t0 = thread{[&]{
        ev.Wait([]{ cout << "Wait_0(" << GetMilliTimestamp() << ")" << endl; });
    }};

    auto t1 = thread{[&]{
        ev.Wait(
            []{ cout << "Wait_1_Pre(" << GetMilliTimestamp() << ")" << endl; },
            []{ cout << "Wait_1_Post(" << GetMilliTimestamp() << ")" << endl;}
        );
    }};

    auto t2 = thread{[&]{
        if (!ev.WaitFor(
            1s,
            []{ cout << "WaitFor_2_Pre(" << GetMilliTimestamp() << ")" << endl; },
            []{ cout << "WaittFor_2_Post(" << GetMilliTimestamp() << ")" << endl;}
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
