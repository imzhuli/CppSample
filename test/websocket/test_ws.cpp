#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/WebSocket.hpp>
#include <zec/Util/Chrono.hpp>
#include <zec/String.hpp>
#include <iostream>
#include <fstream>

using namespace zec;
using namespace std;

xIoContext IoContext;
static std::string ServerIp = "192.168.123.44";
static uint16_t    ServerPort = 8002;
static std::string Hostname = "www.baidu.com";
static std::string Path = "/device";

static uint64_t RequestIdConter = 1024;
static std::string TestJson0_  = R"({"requestId":)";
static std::string TestJson0__ = R"(, "head":1004, "body":{ "userName":"101114653296-Kaie1cmC", "passWord":"948f8c8e81d8c5800e40cafdb7481aae" }})";
static std::string TestJson1_  = R"({"requestId":)";
static std::string TestJson1__ = R"(, "head":1004, "body":{ "userName":"101114653296-EPVvFYLs", "passWord":"76111148810f6fb6efe1f0da9db68c92" }})";

class xWSTest
: public xWebSocketSession::iListener
{
private:
    void OnConnected(xWebSocketSession * WebSocketClientPtr) {
        cout << "WS Connected" << endl;
    }
    void OnHandshakeDone(xWebSocketSession * WebSocketClientPtr) {
        cout << "WS Handshake done" << endl;
        WSClient.PostTextData(TestJson0_ + std::to_string(RequestIdConter++) + TestJson0__);
        WSClient.PostTextData(TestJson1_ + std::to_string(RequestIdConter++) + TestJson1__);
    }
    void OnMessage(xWebSocketSession * WebSocketClientPtr, bool Binary, const void * DataPtr, size_t DataSize) {
        cout << "ReceiveData(Text=" << YN(!Binary) << "): " << string_view((const char *)DataPtr, DataSize) << endl;
     }
    void OnError(xWebSocketSession * WebSocketClientPtr) {
        cout << "Error" << endl;// crash here
    }

    xWebSocketSession WSClient;

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
    xWSTest                Tester1;
    xWSTest                Tester2;
    xWSTest                Tester3;

    if (!IoContext.Init()) {
        throw "Failed to init io context";
    }
    if (!Tester.Init()) {
        throw "Failed to init test object";
    }
    if (!Tester1.Init()) {
        throw "Failed to init test object";
    }
    if (!Tester2.Init()) {
        throw "Failed to init test object";
    }
    if (!Tester3.Init()) {
        throw "Failed to init test object";
    }

    Tester1.Clean();
    Tester1.Init();

    xTimer Timeup;
    while(!Timeup.TestAndTag(5s)) {
        IoContext.LoopOnce(100);
    }

    Tester3.Clean();
    Tester2.Clean();
    Tester1.Clean();
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