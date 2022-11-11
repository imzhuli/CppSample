#include <xel_ext/IO/TcpConnection.hpp>
#include <cinttypes>

#if defined(X_SYSTEM_DARWIN)

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        Fatal("Not implemented");
        return false;
    }

    void xTcpConnection::Clean()
    {
        Fatal("Not implemented");
    }

    size_t xTcpConnection::PostData(const void * DataPtr, size_t DataSize)
    {
        Fatal("Not implemented");
        return 0;
    }

}

#endif
