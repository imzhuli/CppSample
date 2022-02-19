#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/WebSocket.hpp>
#include <zec/Util/Chrono.hpp>
#include <zec/String.hpp>
#include <iostream>
#include <fstream>

using namespace zec;
using namespace std;

xIoContext IoContext;
static std::string ServerIp = "192.168.123.36";
static uint16_t    ServerPort = 8002;
static std::string Hostname = "www.baidu.com";
static std::string Path = "/device";

static std::string TestJson0 = R"({"requestId":0, "head":1004, "body":{ "userName":"101114653296-Kaie1cmC", "passWord":"948f8c8e81d8c5800e40cafdb7481aae" }})";
static std::string TestJson1 = R"({"requestId":1, "head":1004, "body":{ "userName":"101114653296-EPVvFYLs", "passWord":"76111148810f6fb6efe1f0da9db68c92" }})";

class xWSTest
: public xWebSocketClient::iListener
{
private:
    void OnConnected(xWebSocketClient * WebSocketClientPtr) {
        cout << "WS Connected" << endl;
    }
    void OnHandshakeDone(xWebSocketClient * WebSocketClientPtr) {
        cout << "WS Handshake done" << endl;
        WSClient.PostData(TestJson0);
        WSClient.PostData(TestJson1);
    }
    void OnMessage(xWebSocketClient * WebSocketClientPtr, bool Binary, const void * DataPtr, size_t DataSize) {
        cout << "ReceiveData(Text=" << YN(!Binary) << "): " << string_view((const char *)DataPtr, DataSize) << endl;
     }
    void OnError(xWebSocketClient * WebSocketClientPtr) {
        cout << "Error" << endl;// crash here
    }

    xWebSocketClient WSClient;

public:
    bool Init() {
        WSClient.Init(&IoContext, ServerIp.c_str(), ServerPort, Hostname, Path, this);
        return true;
    }

    void Clean() {
        WSClient.Clean();
    }
};

void test0 ()
{
    xWSTest                Tester;

    if (!IoContext.Init()) {
        throw "Failed to init io context";
    }
    if (!Tester.Init()) {
        throw "Failed to init test object";
    }

    xTimer Timer;
    while(!Timer.TestAndTag(2s)) {
        IoContext.LoopOnce(100);
    }

    Tester.Clean();
    IoContext.Clean();
    cout << "IoContext cleaned" << endl;
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