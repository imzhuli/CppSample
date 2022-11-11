#include <xel_ext/IO/TcpServer.hpp>

#if defined(X_SYSTEM_DARWIN)

#include <cinttypes>
X_NS
{

    bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort)
    {
        Fatal("Not implemented");
        return false;
    }

    void xTcpServer::Clean()
    {
        Fatal("Not implemented");
    }



}

#endif
