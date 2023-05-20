#include <xel_ext/IO/UdpChannel.hpp>

#ifdef X_SYSTEM_WINDOWS

X_NS
{

	bool xUdpChannel::Init(xIoContext * IoContextPtr, int AddressFamily, iListener * ListenerPtr)
	{
		assert(IoContextPtr);
		assert(ListenerPtr);
		_Socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (_Socket == InvalidSocket) {
			return false;
		}

		_ListenerPtr = ListenerPtr;
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

		_ListenerPtr = ListenerPtr;
		SocketGuard.Dismiss();
		return false;
	}

	void xUdpChannel::Clean()
	{
		assert(_ListenerPtr);
		assert(_Socket != InvalidSocket);

		XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
		X_DEBUG_RESET(_ListenerPtr);
	}

	void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & Address)
	{
        sockaddr_storage AddrStorage = {};
        size_t AddrLen = Address.Dump(&AddrStorage);
		sendto(_Socket, (const char *)DataPtr, (int)DataSize, 0, (const sockaddr*)&AddrStorage, (socklen_t)AddrLen);
	}

    void xUdpChannel::OnIoEventInReady()
	{
		Todo();
	}

    void xUdpChannel::OnIoEventError()
	{
		_ListenerPtr->OnError(this);
	}

}

#endif
