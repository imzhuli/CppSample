#include <iostream>
#include <asio/asio.hpp>

using namespace std;

void TimeoutCallback(const std::error_code&)
{
    cout << "TimerReached" << endl;
}

int main(int, char **)
{
    cout << "program started" << endl;

    asio::io_context IOCtx;
    asio::steady_timer t(IOCtx, 1s);
    t.async_wait(TimeoutCallback);
    IOCtx.run();
    IOCtx.run();
    cout << "program ended" << endl;
    return 0;
}
