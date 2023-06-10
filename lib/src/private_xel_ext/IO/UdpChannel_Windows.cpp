#include <xel_ext/IO/UdpChannel.hpp>
#include <xel/String.hpp>

#ifdef X_SYSTEM_WINDOWS

X_NS
{

	bool xUdpChannel::Init(xIoContext * IoContextPtr, int AddressFamily, iListener * ListenerPtr)
	{
		if (AddressFamily == AF_INET) {
			return Init(IoContextPtr, xNetAddress::Make4(), ListenerPtr);
		}
		else if (AddressFamily == AF_INET6) {
			return Init(IoContextPtr, xNetAddress::Make6(), ListenerPtr);
		}
		return false;
	}

	bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr)
	{
		assert(IoContextPtr);
		assert(ListenerPtr);

		_IoContextPtr = IoContextPtr;
		_ListenerPtr = ListenerPtr;
		_ErrorProcessed = false;

		_Socket = socket(BindAddress.GetAddressFamily(), SOCK_DGRAM, 0);
		if (_Socket == InvalidSocket) {
			return false;
		}
		auto SocketGuard = xScopeGuard([&]{ XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket)); });

        sockaddr_storage AddrStorage;
        size_t AddrLen = BindAddress.Dump(&AddrStorage);
        auto BindRet = bind(_Socket, (sockaddr*)&AddrStorage, (int)AddrLen);
        if( BindRet == SOCKET_ERROR ) {
            return false;
        }

        if (CreateIoCompletionPort((HANDLE)_Socket, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xUdpChannel::Init failed to create competion port\n");
            return false;
        }

        if (!(_IoBufferPtr = CreateOverlappedObject())) { return false; }
        auto OverlappedObjectGuard = xScopeGuard([this]{ ReleaseOverlappedObject(_IoBufferPtr); });

		TryRecvData();
		if (HasError()) {
			return false;
		}

		OverlappedObjectGuard.Dismiss();
		SocketGuard.Dismiss();
		SetAvailable();

		return true;
	}

	void xUdpChannel::Clean()
	{
		assert(_ListenerPtr);
		assert(_Socket != InvalidSocket);
		assert(_IoBufferPtr);

        _IoBufferPtr->DeleteMark = true;
		ReleaseOverlappedObject(X_DEBUG_STEAL(_IoBufferPtr));

		XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
		X_DEBUG_RESET(_ListenerPtr);
		X_DEBUG_RESET(_IoContextPtr);
	}

	void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & Address)
	{
        sockaddr_storage AddrStorage = {};
        size_t AddrLen = Address.Dump(&AddrStorage);
		sendto(_Socket, (const char *)DataPtr, (int)DataSize, 0, (const sockaddr*)&AddrStorage, (socklen_t)AddrLen);
	}

	void xUdpChannel::TryRecvData()
	{
		auto & ReadObject = _IoBufferPtr->ReadObject;
		if (ReadObject.AsyncOpMark) {
			return;
		}

		auto & BU = ReadObject.BufferUsage;
		BU.buf = (CHAR*)_IoBufferPtr->ReadBuffer;
		BU.len = (ULONG)sizeof(_IoBufferPtr->ReadBuffer);
		memset(&_RemoteAddress, 0, sizeof(_RemoteAddress));
		_RemoteAddressLength = sizeof(_RemoteAddress);
		memset(&ReadObject.NativeOverlappedObject, 0, sizeof(ReadObject.NativeOverlappedObject));
		auto Error = WSARecvFrom(_Socket, &BU, 1, nullptr, X2Ptr(DWORD(0)), (sockaddr*)&_RemoteAddress, &_RemoteAddressLength, &ReadObject.NativeOverlappedObject, nullptr);
		if (Error) {
			auto ErrorCode = WSAGetLastError();
			if (ErrorCode != WSA_IO_PENDING) {
				X_DEBUG_PRINTF("WSARecvFrom ErrorCode: %u\n", ErrorCode);
				SetError();
				return;
			}
		}
		ReadObject.AsyncOpMark = true;
		RetainOverlappedObject(_IoBufferPtr);
	}

    void xUdpChannel::OnIoEventInReady()
	{
		auto & ReadObject = _IoBufferPtr->ReadObject;
		ReadObject.AsyncOpMark = false;

		auto RemoteAddress = xNetAddress::Parse((sockaddr*)&_RemoteAddress);
		_ListenerPtr->OnData(this, _IoBufferPtr->ReadBuffer, _IoBufferPtr->ReadObject.DataSize, RemoteAddress);

		TryRecvData();
	}

    void xUdpChannel::OnIoEventError()
	{
		if (!Steal(_ErrorProcessed, true)) {
			_ListenerPtr->OnError(this);
		}
	}

}

#endif
