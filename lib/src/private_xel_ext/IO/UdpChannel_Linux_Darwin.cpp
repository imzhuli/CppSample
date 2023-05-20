#include <xel_ext/IO/UdpChannel.hpp>

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)

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
        if (BindRet == -1) {
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
		sendto(_Socket, (const char *)DataPtr, DataSize, 0, (const sockaddr*)&AddrStorage, (socklen_t)AddrLen);
	}

    void xUdpChannel::OnIoEventInReady()
	{
		// xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress)
		ubyte Buffer[MaxPacketSize];
		sockaddr_storage SockAddr;
		socklen_t SockAddrLen = sizeof(SockAddr);
		int ReadSize = recvfrom(_Socket, Buffer, sizeof(Buffer), 0, (sockaddr*)&SockAddr, &SockAddrLen);
		if (ReadSize == -1) {
			if (errno != EAGAIN) {
				SetUnavailable();
				return;
			}
			return;
		}
		xNetAddress RemoteAddress = xNetAddress::Parse((const sockaddr*)&SockAddr);
		_ListenerPtr->OnData(this, Buffer, (size_t)ReadSize, RemoteAddress);
	}

    void xUdpChannel::OnIoEventError()
	{
		_ListenerPtr->OnError(this);
	}

}

#endif