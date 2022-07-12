#include <zec/Util/Command.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace zec;

static bool Silent = false;
static auto Tag = std::string{"TCP_ECHO"};
static auto DeleteQueue = std::vector<xTcpConnection*>{};

static struct : xTcpConnection::iListener
{
    size_t  OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override 
    {
        if (!Silent) {
            TcpConnectionPtr->PostData(DataPtr, DataSize);
        }
        return DataSize;
    }
    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override
    {
        DeleteQueue.push_back(TcpConnectionPtr);
    }
    void OnError(xTcpConnection * TcpConnectionPtr) override
    {
        DeleteQueue.push_back(TcpConnectionPtr);
    }
} TcpReactor;

static struct : xTcpServer::iListener
{
    void OnNewConnection(xTcpServer * TcpServerPtr, xIoHandle NativeHandle) {
        cout << Tag << ": New Connection" << endl;
        auto TcpPtr = new xTcpConnection();
        TcpPtr->Init(NativeHandle, &TcpReactor);
    }
} TcpServerListener;

int main(int argc, char *argv[])
{
    auto Cmd = xCommandLine{ argc, argv, {
        { 's', nullptr, "silent", false }
    }};

    auto OptSlient = Cmd["silent"];
    Silent = OptSlient();

    if (Cmd.GetArgCount() > 1) {
        Tag = Cmd[1];
    }
    cout << "UpdateTag: " << Tag.c_str() << endl;

    auto IoContext = xIoContext{};
    auto IoContextGuard = xResourceGuardThrowable { IoContext };
    
    xTcpServer TcpServer;
    auto TcpServerGuard = xResourceGuardThrowable { TcpServer, &IoContext, "0.0.0.0", 8888, &TcpServerListener };
    
    while(true) {
        IoContext.LoopOnce(100);
        for (auto & Iter : DeleteQueue) {
            Iter->Clean();
            delete(Steal(Iter));
        }
        Renew(DeleteQueue);
    }

    return 0;
}
