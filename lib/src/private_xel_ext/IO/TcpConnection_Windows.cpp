#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>
#include <atomic>
#include <mutex>
#include <cinttypes>

#if defined(X_SYSTEM_WINDOWS)

X_NS
{
    std::atomic<LPFN_CONNECTEX> AtomicConnectEx = nullptr;
    std::atomic<LPFN_CONNECTEX> AtomicConnectEx6 = nullptr;
    std::mutex ConnectExLoaderMutex;

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        assert(NativeHandle != InvalidSocket);
        assert(!_ListenerPtr && ListenerPtr);
        X_DEBUG_PRINTF("xTcpConnection::Init IoContextPtr=%p, Socket=%" PRIuPTR "\n", IoContextPtr, (uintptr_t&)NativeHandle);

        if (CreateIoCompletionPort((HANDLE)NativeHandle, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to create competion port\n");
            return false;
        }

        memset(&_ReadOverlappedObject, 0, sizeof(_ReadOverlappedObject));
        memset(&_WriteOverlappedObject, 0, sizeof(_WriteOverlappedObject));

        _WriteBufferPtr = nullptr;
        _Socket = NativeHandle;
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _Status = eStatus::Connected;
        _SuspendReading = false;
        _Reading = false;

        TryRecvData();
        if (!IsAvailable()) {
            return false;
        }
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        int AF = AF_UNSPEC;
        sockaddr_storage AddrStorage;
        size_t AddrLen = Address.Dump(&AddrStorage);
        if (Address.IsV4()) {
            AF = AF_INET;
        } else if (Address.IsV6()) {
            AF = AF_INET6;
        }
        else {
            return false;
        }

        assert(_Socket == InvalidSocket);
        _Socket = InvalidSocket;

        auto FailSafe = xScopeGuard{[this]{
            if (_Socket != InvalidSocket) {
                XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
            }
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        _Socket = WSASocket(AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_Socket == InvalidSocket) {
            return false;
        }

        // load connect ex:
        LPFN_CONNECTEX ConnectEx = nullptr;
        if (AF == AF_INET) {
            ConnectEx = AtomicConnectEx.load();
        } else if (AF == AF_INET6) {
            ConnectEx = AtomicConnectEx6.load();
        } else {
            Fatal("Bug");
        }
        if (!ConnectEx) {
            auto LockGuard = std::lock_guard(ConnectExLoaderMutex);
            GUID guid = WSAID_CONNECTEX;
            DWORD dwBytes = 0;
            auto LoadError = WSAIoctl(_Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                        &guid, sizeof(guid),
                        &ConnectEx, sizeof(ConnectEx),
                        &dwBytes, NULL, NULL);
            if (LoadError) {
                auto ErrorCode = WSAGetLastError();
                if (ErrorCode != WSA_IO_PENDING) {
                    X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
                    SetUnavailable();
                }
                return false;
            }
            X_DEBUG_PRINTF("ConnectEx: %p\n", ConnectEx);

            if (AF == AF_INET) {
                AtomicConnectEx = ConnectEx;
            } else if (AF == AF_INET6) {
                AtomicConnectEx6 = ConnectEx;
            } else {
                Fatal("Bug");
            }
        }while(false);

        if (CreateIoCompletionPort((HANDLE)_Socket, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to create competion port\n");
            return false;
        }

        memset(&_ReadOverlappedObject, 0, sizeof(_ReadOverlappedObject));
        memset(&_WriteOverlappedObject, 0, sizeof(_WriteOverlappedObject));

        _WriteBufferPtr = nullptr;
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        /* ConnectEx requires the socket to be initially bound. */
        do {
            struct sockaddr_storage BindAddr;
            memset(&BindAddr, 0, sizeof(BindAddr));
            BindAddr.ss_family = AddrStorage.ss_family;
            auto Error = bind(_Socket, (SOCKADDR*) &BindAddr, (int)AddrLen);
            if (Error) {
                X_DEBUG_PRINTF("bind failed: %u\n", WSAGetLastError());
                return false;
            }
        } while(false);

        auto Error = ConnectEx(_Socket, (SOCKADDR*)(&AddrStorage), (int)AddrLen, NULL, NULL, NULL, &_WriteOverlappedObject);
        if (Error) {
            auto ErrorCode = WSAGetLastError();
            X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
            return false;
        }
        _Status = eStatus::Connecting;
        _SuspendReading = false;

        FailSafe.Dismiss();
        return true;
    }

    void xTcpConnection::Clean()
    {
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
        X_DEBUG_RESET(_ListenerPtr);
        X_DEBUG_RESET(_Status);
    }

    void xTcpConnection::OnIoEventInReady()
    {
        _Reading = false;
        if (!_ReadDataSize) {
            _Status = eStatus::Closed;
            _ListenerPtr->OnPeerClose(this);
            return;
        }
        size_t TotalSize = (_ReadBufferUsage.buf - (CHAR*)_ReadBuffer) + _ReadDataSize;
        size_t SkipSize  = TotalSize - _ListenerPtr->OnData(this, _ReadBuffer, TotalSize);
        TryRecvData(SkipSize);
    }

    void xTcpConnection::OnIoEventOutReady()
    {
        if (_Status == eStatus::Connecting) {
            int seconds;
            int bytes = sizeof(seconds);

            auto iResult = getsockopt(_Socket, SOL_SOCKET, SO_CONNECT_TIME,
                                (char *)&seconds, (PINT)&bytes );
            if (iResult != NO_ERROR ) {
                X_DEBUG_PRINTF( "getsockopt(SO_CONNECT_TIME) failed with error: %u\n", WSAGetLastError());
                SetUnavailable();
                return;
            }
            else {
                if (seconds == -1) {
                    X_DEBUG_PRINTF("Connection not established yet\n");
                    SetUnavailable();
                    return;
                }
                X_DEBUG_PRINTF("Connection has been established %u seconds\n", seconds);
            }
            _Status = eStatus::Connected;
            _ListenerPtr->OnConnected(this);

            assert(!_Reading);
            TryRecvData();
            if (!IsAvailable()) {
                return;
            }
        }
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

    void xTcpConnection::TryRecvData(size_t SkipSize)
    {
        if (_SuspendReading) {
            return;
        }
        _Reading = true;
        _ReadBufferUsage.buf = (CHAR*)_ReadBuffer + SkipSize;
        _ReadBufferUsage.len = (ULONG)(sizeof(_ReadBuffer) - SkipSize);
        _ReadFlags = 0;
        assert(_ReadBufferUsage.len);
        auto Error = WSARecv(_Socket, &_ReadBufferUsage, 1, nullptr, &_ReadFlags, &_ReadOverlappedObject, nullptr);
        if (Error) {
            auto ErrorCode = WSAGetLastError();
            if (ErrorCode != WSA_IO_PENDING) {
                X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
                SetUnavailable();
            }
        }
    }

    void xTcpConnection::TrySendData()
    {
        if (_Status == eStatus::Connecting) {
            return;
        }
        if (_WriteBufferPtr) {
            assert(_WriteBufferUsage.len == (ULONG)_WriteBufferPtr->DataSize);
            delete _WriteBufferPtr;
        }
        _WriteBufferPtr = _WriteBufferChain.Pop();
        if (!_WriteBufferPtr) {
            return;
        }
        _WriteBufferUsage.buf = (CHAR*)_WriteBufferPtr->Buffer;
        _WriteBufferUsage.len = (ULONG)_WriteBufferPtr->DataSize;
        auto Error = WSASend(_Socket, &_WriteBufferUsage, 1, nullptr, 0, &_WriteOverlappedObject, nullptr);
        if (Error) {
            auto ErrorCode = WSAGetLastError();
            if (ErrorCode != WSA_IO_PENDING) {
                X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
                SetUnavailable();
            }
        }
        return;
    }

    void xTcpConnection::SuspendReading()
    {
        if (_SuspendReading) {
            return;
        }
        _SuspendReading = true;
    }

    void xTcpConnection::ResumeReading()
    {
        if (!_SuspendReading) {
            return;
        }
        _SuspendReading = false;
        if (!_Reading) {
            TryRecvData();
        }
    }

}

#endif
