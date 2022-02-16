#include <zec_ext/WebSocket/WS.hpp>
#include <iostream>

using namespace zec;
using namespace std;

static bool Stop = false;

class TestWS
: public xWebSocketClient
{
    bool OnWSDataIn(const void * DataPtr, size_t DataSize)
    {
        std::string S { (const char *)DataPtr, DataSize };
        cout << "Data: " << S << endl;
        if (S == "echo") {
            std::string HW = "Hello world!";
            Post(HW.data(), HW.length());
            return true;
        }
        else if (S == "quit") {
            Stop = true;
            return false;
        } else if (S == "disconnect") {
            return false;
        }
        return true;
    }

    void OnWSDisconnected() override
    {
        cout << "Disconnected" << endl;
    }
};

static xWebSocketContext Context;
static TestWS Client;

int main(int, char **)
{
    if (!Context.Init()) {
        return -1;
    }

    xWebSocketClient::xConfig Config = {};
    Config.Hostname = "192.168.123.62";
    Config.Port = 8443;
    Config.Path = "v1";

    Client.Init(&Context, Config);
    while(!Stop)
    {
        if (!Client.IsConnected()) {
            Client.Clean();
            Client.Init(&Context, Config);
        }
        Context.LoopOnce(1000);
    }
    Client.Clean();

}
