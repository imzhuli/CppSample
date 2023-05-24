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
		TryRecvData();

		_ListenerPtr = ListenerPtr;
		SocketGuard.Dismiss();
		SetAvailable();
		return true;
	}

	void xUdpChannel::Clean()
	{
		assert(_ListenerPtr);
		assert(_Socket != InvalidSocket);

		X_DEBUG_RESET(_ListenerPtr);
		XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
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
        _ReadBufferUsage.buf = (CHAR*)_ReadBuffer;
        _ReadBufferUsage.len = (ULONG)sizeof(_ReadBuffer);
		_ReadFlags = 0;
		memset(&_RemoteAddress, 0, sizeof(_RemoteAddress));
		_RemoteAddressLength = sizeof(_RemoteAddress);
        memset(&_ReadOverlappedObject, 0, sizeof(_ReadOverlappedObject));
        auto Error = WSARecvFrom(_Socket, &_ReadBufferUsage, 1, nullptr, &_ReadFlags, (sockaddr*)&_RemoteAddress, &_RemoteAddressLength, &_ReadOverlappedObject, nullptr);
        if (Error) {
            auto ErrorCode = WSAGetLastError();
            if (ErrorCode != WSA_IO_PENDING) {
                X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
                SetUnavailable();
            }
        }
	}

    void xUdpChannel::OnIoEventInReady()
	{
		assert(_ReadDataSize);
		auto RemoteAddress = xNetAddress::Parse((sockaddr*)&_RemoteAddress);
		_ListenerPtr->OnData(this, _ReadBuffer, _ReadDataSize, RemoteAddress);
		TryRecvData();
	}

    void xUdpChannel::OnIoEventError()
	{
		_ListenerPtr->OnError(this);
	}

	eIoEventType xUdpChannel::GetEventType(OVERLAPPED * OverlappedPtr)
	{
		return eIoEventType::InReady;
	}

}

#endif
