#include <xel_ext/IO/IoContext.hpp>
#include <xel_ext/IO/TcpServer.hpp>
#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/Util/Chrono.hpp>
#include <xel/String.hpp>
#include <thread>

using namespace xel;

std::vector<xTcpConnection*> DeleteList;

struct xSample
: xTcpServer::iListener
, xTcpConnection::iListener
{
    void OnNewConnection(xTcpServer * TcpServerPtr, xSocket NativeHandle) override
    {
        X_DEBUG_PRINTF("xSample: New connection accepted: handle=%p\n", (void*)NativeHandle);
        auto ConnectionPtr = new xTcpConnection;
        if (!ConnectionPtr->Init(TcpServerPtr->GetIoContextPtr(), NativeHandle, this)) {
            XelCloseSocket(NativeHandle);
            delete ConnectionPtr;
            return;
        }
    }

    size_t OnData(xTcpConnection * ConnectionPtr, void * DataPtr, size_t DataSize) override 
    {        
        auto Hex = HexShow(DataPtr, DataSize);
        X_DEBUG_PRINTF("xSample::OnData Instance=%p\nData=\n%s\n", this, Hex.c_str());
        ConnectionPtr->PostData("HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 57);
        return DataSize;
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override
    {
        TcpConnectionPtr->Clean();
        X_DEBUG_PRINTF("xSample::OnPeerClose Instance=%p\n", this);
        DeleteList.push_back(TcpConnectionPtr);
    }

} SampleListener;

int main(int, char **)
{
    xIoContext IoContext;
    auto IoContextGuard = xResourceGuard{ IoContext };

    xNetAddress Address = xNetAddress::Parse("0.0.0.0", 8080);
    xTcpServer TcpServer;
    auto TcpServerGuard = xResourceGuard{ TcpServer, &IoContext, Address, &SampleListener };


    xTimer Timer;
    while(true)
    {
        IoContext.LoopOnce();
        for (auto & ConnectionPtr : DeleteList) {
            delete ConnectionPtr;
        }
        DeleteList.clear();        
    }


    return 0;
}
