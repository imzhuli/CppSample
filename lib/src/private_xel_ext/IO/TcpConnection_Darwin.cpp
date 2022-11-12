#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>
#include <cinttypes>
#include <fcntl.h>

#if defined(X_SYSTEM_DARWIN)

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        assert(NativeHandle != InvalidSocket);
        X_DEBUG_PRINTF("xTcpConnection::Init NewConnection poller=%i socket=%i\n", (int)*IoContextPtr, NativeHandle);

        int flags = fcntl(NativeHandle, F_GETFL);
        fcntl(NativeHandle, F_SETFL, flags | O_NONBLOCK);

        struct kevent Event = {};
        Event.ident = NativeHandle;
        Event.flags = EV_ADD | EV_CLEAR;
        Event.filter = EVFILT_READ;
        Event.udata = this;
        if (-1 == kevent(*IoContextPtr, &Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to register kevent\n");
            return false;
        }

        _Socket = NativeHandle;
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _ReadBufferDataSize = 0;
        _WriteBufferDataSize = 0;
        _SendingBufferPtr = nullptr;
        _Status = eStatus::Connected;
        return true;
    }

    void xTcpConnection::Clean()
    {
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
    }

    void xTcpConnection::OnIoEventInReady()
    {
        size_t TotalSpace = sizeof(_ReadBuffer) - _ReadBufferDataSize;
        assert(TotalSpace);

        int ReadSize = read(_Socket, _ReadBuffer + _ReadBufferDataSize, TotalSpace);
        if (0 == ReadSize) {
            X_DEBUG_PRINTF("xTcpConnection::OnIoEventInReady EOF\n");
            _Status = eStatus::Closed;
            _ListenerPtr->OnPeerClose(this);
            return;
        }
        if (-1 == ReadSize) {
            if (EAGAIN == ReadSize) {
                return;
            }
            SetUnavailable();
            return;
        }
        _ReadBufferDataSize += ReadSize;
        _ReadBufferDataSize -= _ListenerPtr->OnData(this, _ReadBuffer, _ReadBufferDataSize);
        return;
    }

    void xTcpConnection::OnIoEventOutReady()
    {
        TrySendData();
    }

    size_t xTcpConnection::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataPtr_ && DataSize);
        assert(_Status != eStatus::Unspecified);

        if (_Status >= eStatus::Closing) {
            return 0;
        }

        auto DataPtr = (const ubyte*)DataPtr_;
        auto Packets = DataSize / sizeof(xPacketBuffer::Buffer);
        for (size_t i = 0 ; i < Packets; ++i) {
            auto BufferPtr = new xPacketBuffer;
            memcpy(BufferPtr->Buffer, DataPtr, sizeof(xPacketBuffer::Buffer));
            BufferPtr->DataSize = sizeof(xPacketBuffer::Buffer);
            DataPtr  += sizeof(xPacketBuffer::Buffer);
            DataSize -= sizeof(xPacketBuffer::Buffer);
            _WriteBufferChain.Push(BufferPtr);
        }
        if (DataSize) {
            auto BufferPtr = new xPacketBuffer;
            memcpy(BufferPtr->Buffer, DataPtr, DataSize);
            BufferPtr->DataSize = DataSize;
            _WriteBufferChain.Push(BufferPtr);
        }

        if (_Status == eStatus::Connecting) {
            return DataSize;
        }

        // _Status == eStatus::Connected
        TrySendData();
        return DataSize;
    }

    void xTcpConnection::TrySendData()
    {
        if (!_SendingBufferPtr) {
            _SendingBufferPtr = _WriteBufferChain.Pop();
        }
        while(_SendingBufferPtr) {
            ssize_t SendSize = write(_Socket, _SendingBufferPtr->Buffer, _SendingBufferPtr->DataSize);
            if (SendSize == -1) {
                if (errno == EAGAIN) {
                    Fatal("Not implemented");
                    return;
                }
                SetUnavailable();
                return;
            }
            size_t RemainSize = _SendingBufferPtr->DataSize - SendSize;
            if (RemainSize) {
                memmove(_SendingBufferPtr->Buffer, _SendingBufferPtr->Buffer + SendSize, RemainSize);
                Fatal("Check if output event is required");
                return;
            }
            _SendingBufferPtr = _WriteBufferChain.Pop();
        }
        return;
    }

}

#endif
