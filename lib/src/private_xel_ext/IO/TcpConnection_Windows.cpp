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

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket && NativeHandle, iListener * ListenerPtr)
    {
        assert(NativeHandle != InvalidSocket);
        assert(!_ListenerPtr && ListenerPtr);
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        auto FailSafe = xScopeGuard{[=]{
            XelCloseSocket(NativeHandle);
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        #ifdef SO_REUSEADDR
        setsockopt(NativeHandle, SOL_SOCKET, SO_REUSEADDR, (char *)X2Ptr(int(1)), sizeof(int));
        #endif

        if (CreateIoCompletionPort((HANDLE)NativeHandle, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to create competion port\n");
            return false;
        }

        if (!(_IoBufferPtr = CreateOverlappedObject())) { return false; }
        auto OverlappedObjectGuard = xScopeGuard([this]{ ReleaseOverlappedObject(_IoBufferPtr); });

        _Socket = NativeHandle;
        _Status = eStatus::Connected;
        _SuspendReading = false;
        _HasPendingWriteFlag = false;

		TryRecvData();
		if (HasError()) {
			return false;
		}

        OverlappedObjectGuard.Dismiss();
        FailSafe.Dismiss();
        SetAvailable();

        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        int AF = AF_UNSPEC;
        sockaddr_storage AddrStorage = {};
        size_t AddrLen = Address.Dump(&AddrStorage);
        if (Address.IsV4()) {
            AF = AF_INET;
        } else if (Address.IsV6()) {
            AF = AF_INET6;
        }
        else {
            X_DEBUG_PRINTF("Invalid target address\n");
            return false;
        }

        assert(_Socket == InvalidSocket);
        _Socket = WSASocket(AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_Socket == InvalidSocket) {
            X_DEBUG_PRINTF("Failed to create socket\n");
            return false;
        }

        auto FailSafe = xScopeGuard{[this]{
            XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        #ifdef SO_REUSEADDR
        setsockopt(_Socket, SOL_SOCKET, SO_REUSEADDR, (char *)X2Ptr(int(1)), sizeof(int));
        #endif

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
                    return false;
                }
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

        if (!(_IoBufferPtr = CreateOverlappedObject())) { return false; }
        auto OverlappedObjectGuard = xScopeGuard([this]{ ReleaseOverlappedObject(_IoBufferPtr); });

		auto & WriteObject = _IoBufferPtr->WriteObject;
		memset(&WriteObject.NativeOverlappedObject, 0, sizeof(WriteObject.NativeOverlappedObject));
        auto Success = ConnectEx(_Socket, (SOCKADDR*)(&AddrStorage), (int)AddrLen, NULL, NULL, NULL, &WriteObject.NativeOverlappedObject);
        if (!Success) {
            auto ErrorCode = WSAGetLastError();
            if (ErrorCode != ERROR_IO_PENDING) {
                X_DEBUG_PRINTF("Failed to build connection ErrorCode: %u (ERROR_IO_PENDING == 997L)\n", ErrorCode);
                return false;
            }
        }
		_IoBufferPtr->WriteObject.AsyncOpMark = true;
        RetainOverlappedObject(_IoBufferPtr);

        _Status = eStatus::Connecting;
        _SuspendReading = false;
        _HasPendingWriteFlag = false;

        /***
         *  Always wait for completion event, and call TryRecvData Later
         * */
		// TryRecvData();
		// if (HasError()) {
		// 	return false;
		// }

        OverlappedObjectGuard.Dismiss();
        FailSafe.Dismiss();
        SetAvailable();

        return true;
    }

    void xTcpConnection::Clean()
    {
        _IoBufferPtr->DeleteMark = true;
        ReleaseOverlappedObject(X_DEBUG_STEAL(_IoBufferPtr));
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
    }

    size_t xTcpConnection::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataPtr_);
        assert(_Status != eStatus::Unspecified);
        auto & _WriteBufferChain = _IoBufferPtr->WriteBufferChain;

        if (!IsAvailable() || !DataSize) {
            return 0;
        }

        auto DataPtr = (const ubyte*)DataPtr_;
        auto RemainedSize = DataSize;
        auto Packets = RemainedSize / sizeof(xPacketBuffer::Buffer);
        for (size_t i = 0 ; i < Packets; ++i) {
            auto BufferPtr = new (std::nothrow) xPacketBuffer;
            if (!BufferPtr) {
                return DataSize - RemainedSize;
            }
            memcpy(BufferPtr->Buffer, DataPtr, sizeof(xPacketBuffer::Buffer));
            BufferPtr->DataSize = sizeof(xPacketBuffer::Buffer);
            DataPtr  += sizeof(xPacketBuffer::Buffer);
            RemainedSize -= sizeof(xPacketBuffer::Buffer);
            _WriteBufferChain.Push(BufferPtr);
        }
        if (RemainedSize) {
            auto BufferPtr = new (std::nothrow) xPacketBuffer;
            if (!BufferPtr) {
                return DataSize - RemainedSize;
            }
            memcpy(BufferPtr->Buffer, DataPtr, RemainedSize);
            BufferPtr->DataSize = RemainedSize;
            _WriteBufferChain.Push(BufferPtr);
        }

        _HasPendingWriteFlag = true;
        if (_Status == eStatus::Connecting) {
            return DataSize;
        }

        TrySendData();
        return DataSize;
    }

    void xTcpConnection::OnIoEventInReady()
    {
		auto & ReadObject = _IoBufferPtr->ReadObject;
		ReadObject.AsyncOpMark = false;

        if (!ReadObject.DataSize) {
            _Status = eStatus::Closing;
            SetDisabled();
            _ListenerPtr->OnPeerClose(this);
            return;
        }

        auto & BU = ReadObject.BufferUsage;
        auto ReadBufferPtr = _IoBufferPtr->ReadBuffer;
        auto ProcessDataPtr = ReadBufferPtr;
        auto & RemainDataSize = _IoBufferPtr->UnprocessedDataSize;
        RemainDataSize += ReadObject.DataSize;
        while(RemainDataSize) {
            auto ProcessedData = _ListenerPtr->OnData(this, ProcessDataPtr, RemainDataSize);
            if (ProcessedData == InvalidPacketSize) {
                SetError();
                return;
            }
            if (!ProcessedData){
                if (ProcessDataPtr != ReadBufferPtr) { // some data are processed
                    memmove(ReadBufferPtr, ProcessDataPtr, RemainDataSize);
                }
                break;
            }
            ProcessDataPtr += ProcessedData;
            RemainDataSize -= ProcessedData;
        }

        TryRecvData();
    }

    void xTcpConnection::OnIoEventOutReady()
    {
		auto & WriteObject = _IoBufferPtr->WriteObject;
		WriteObject.AsyncOpMark = false;

        if (_Status == eStatus::Connecting) {
            int seconds;
            int bytes = sizeof(seconds);

            auto iResult = getsockopt(_Socket, SOL_SOCKET, SO_CONNECT_TIME,
                                (char *)&seconds, (PINT)&bytes );
            if (iResult != NO_ERROR ) {
                X_DEBUG_PRINTF( "getsockopt(SO_CONNECT_TIME) failed with error: %u\n", WSAGetLastError());
                SetError();
                return;
            }
            else {
                if (seconds == -1) {
                    X_DEBUG_PRINTF("Connection not established yet\n");
                    SetError();
                    return;
                }
                // X_DEBUG_PRINTF("Connection has been established %u seconds\n", seconds);
            }
            _Status = eStatus::Connected;

            auto NeedFlushEvent = _HasPendingWriteFlag;
            _ListenerPtr->OnConnected(this);
            TrySendData();
            if (NeedFlushEvent && !_HasPendingWriteFlag) {
                _ListenerPtr->OnFlush(this);
            }
            TryRecvData();
        }
        else {
            // remove last sent data from WriteBufferChain
            auto WriteBufferPtr = _IoBufferPtr->WriteBufferChain.Pop();
            assert(WriteBufferPtr == (void*)Steal(WriteObject.BufferUsage.buf));
            delete WriteBufferPtr;

            auto NeedFlushEvent = _HasPendingWriteFlag;
            TrySendData();
            if (NeedFlushEvent && !_HasPendingWriteFlag) {
                _ListenerPtr->OnFlush(this);
            }
        }
    }

    void xTcpConnection::TryRecvData()
    {
        if (_SuspendReading) {
            return;
        }
		auto & ReadObject = _IoBufferPtr->ReadObject;
		if (ReadObject.AsyncOpMark) {
			return;
		}

        auto UnprocessedDataSize = _IoBufferPtr->UnprocessedDataSize;
        auto RemainSpaceSize = sizeof(_IoBufferPtr->ReadBuffer) - UnprocessedDataSize;
        assert(RemainSpaceSize);

		auto & BU = ReadObject.BufferUsage;
		BU.buf = (CHAR*)(_IoBufferPtr->ReadBuffer + UnprocessedDataSize);
		BU.len = (ULONG)RemainSpaceSize;
		memset(&ReadObject.NativeOverlappedObject, 0, sizeof(ReadObject.NativeOverlappedObject));
        auto Error = WSARecv(_Socket, &BU, 1, nullptr, X2Ptr(DWORD(0)), &ReadObject.NativeOverlappedObject, nullptr);
		if (Error) {
			auto ErrorCode = WSAGetLastError();
			if (ErrorCode != WSA_IO_PENDING) {
				X_DEBUG_PRINTF("WSARecvFrom ErrorCode: %u\n", ErrorCode);
                _IoContextPtr->PostError(*this);
				return;
			}
		}
		ReadObject.AsyncOpMark = true;
		RetainOverlappedObject(_IoBufferPtr);
    }

    void xTcpConnection::TrySendData()
    {
        if (_Status == eStatus::Connecting) {
            return;
        }
		auto & WriteObject = _IoBufferPtr->WriteObject;
		if (WriteObject.AsyncOpMark) {
			return;
		}
        auto WriteBufferPtr = _IoBufferPtr->WriteBufferChain.Peek();
        if (!WriteBufferPtr) {
            _HasPendingWriteFlag = false;
            return;
        }

		auto & BU = WriteObject.BufferUsage;
        BU.buf = (CHAR*)WriteBufferPtr->Buffer;
        BU.len = (ULONG)WriteBufferPtr->DataSize;
        memset(&WriteObject.NativeOverlappedObject, 0, sizeof(WriteObject.NativeOverlappedObject));
        auto Error = WSASend(_Socket, &BU, 1, nullptr, 0, &WriteObject.NativeOverlappedObject, nullptr);
        if (Error) {
            auto ErrorCode = WSAGetLastError();
            if (ErrorCode != WSA_IO_PENDING) {
                X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
                _IoContextPtr->PostError(*this);
                return;
            }
        }
        if (!WriteBufferPtr->HasNext()) {
            _HasPendingWriteFlag = false;
        }
		WriteObject.AsyncOpMark = true;
        RetainOverlappedObject(_IoBufferPtr);
        return;
    }

    void xTcpConnection::SuspendReading()
    {
        _SuspendReading = true;
    }

    void xTcpConnection::ResumeReading()
    {
        _SuspendReading = false;
        if (_Status == eStatus::Connected) {
            TryRecvData();
        }
    }

}

#endif
