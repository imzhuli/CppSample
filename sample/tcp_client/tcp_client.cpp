#include <xel_ext/IO/IoContext.hpp>
#include <xel_ext/IO/TcpServer.hpp>
#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/Util/Chrono.hpp>
#include <xel/String.hpp>
#include <cstdio>
#include <thread>
#include <cinttypes>
#include <iostream>

using namespace xel;
using namespace std;


static const char * host = "14.215.177.38";
// static const char * host = "8.8.8.8";
static const uint16_t port = 80;
static const char * SendData = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nUser-Agent: curl/7.68.0\r\n\r\n";

struct xSample
: xTcpConnection::iListener
{
    void OnConnected(xTcpConnection * ConnectionPtr) override
    {
        printf("xSample::OnConnectionEstablished Instance=%p\n", this);
    }

    size_t OnData(xTcpConnection * ConnectionPtr, void * DataPtr, size_t DataSize) override
    {
        auto Hex = HexShow(DataPtr, DataSize);
        printf("xSample::OnData Instance=%p\nDataSize=%zi\n", this, DataSize);
        return DataSize;
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override
    {
        TcpConnectionPtr->Clean();
        printf("xSample::OnPeerClose Instance=%p\n", this);
        IsClosed = true;
    }

    void OnError(xTcpConnection * TcpConnectionPtr) override
    {
        TcpConnectionPtr->Clean();
        printf("xSample::OnConnectionError Instance=%p\n", this);
        IsClosed = true;
    }

    bool IsClosed = false;
} SampleListener;

int main(int argc, char * argv[])
{
    xIoContext IoContext;
    auto IoContextGuard = xResourceGuard{ IoContext };

    xTcpConnection Connection;
    xNetAddress Address = xNetAddress::Parse(host, port);
    auto ConnectionGuard = xResourceGuard{ Connection, &IoContext, Address, &SampleListener };


    xTcpConnection Connection2;
    auto ConnectionGuard2 = xResourceGuard{ Connection2, &IoContext, Address, &SampleListener };

    if (!ConnectionGuard) {
        cerr << "Failed to init Connection" << endl;
        return 0;
    }

    Connection.PostData(SendData, strlen(SendData));
    Connection2.PostData(SendData, strlen(SendData));

    while(true)
    {
        if (SampleListener.IsClosed) {
            break;
        }
        IoContext.LoopOnce(100);
    }

    return 0;
}