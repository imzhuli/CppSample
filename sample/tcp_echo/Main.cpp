#include <zec/Util/Command.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <iostream>

using namespace std;
using namespace zec;

static auto Tag = std::string{"TCP_ECHO"};

static struct : xTcpServer::iListener
{
    void OnNewConnection(xTcpServer * TcpServerPtr, xIoHandle NativeHandle) {
        cout << Tag << ": New Connection" << endl;
    }
} TcpServerListener;

int main(int argc, char *argv[])
{
    auto Cmd = xCommandLine{ argc, argv, {} };

    if (Cmd.GetArgCount() > 1) {
        Tag = Cmd[1];
    }
    cout << "UpdateTag: " << Tag.c_str() << endl;

    xIoContext IoContext;
    auto IoContextGuard = xResourceGuardThrowable { IoContext };
    
    xTcpServer TcpServer;
    auto TcpServerGuard = xResourceGuardThrowable { TcpServer, &IoContext, "0.0.0.0", 8888, &TcpServerListener };
    
    while(true) {
        IoContext.LoopOnce(100);
    }

    return 0;
}
