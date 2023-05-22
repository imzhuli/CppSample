#include <xel_ext/IO/TcpConnection.hpp>
#include <climits>

X_NS
{

    void xTcpConnection::ResizeSendBuffer(size_t Size)
    {
        assert(Size < INT_MAX);
        if (_Socket == InvalidSocket) {
            return;
        }
        setsockopt(_Socket, SOL_SOCKET, SO_SNDBUF, (char *)X2Ptr(int(Size)), sizeof(int));
    }

    void xTcpConnection::ResizeReceiveBuffer(size_t Size)
    {
        assert(Size < INT_MAX);
        if (_Socket == InvalidSocket) {
            return;
        }
        setsockopt(_Socket, SOL_SOCKET, SO_RCVBUF, (char *)X2Ptr(int(Size)), sizeof(int));
    }

}
