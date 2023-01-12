#include <xel_ext/IO/UdpChannel.hpp>

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)

X_NS
{

	bool xUdpChannel::Init(xIoContext * IoContextPtr, iListener * ListenerPtr)
	{
		assert(IoContextPtr);
		assert(ListenerPtr);
		_ListenerPtr = ListenerPtr;
		return false;
	}

	bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr)
	{
		Todo();

		assert(IoContextPtr);
		assert(ListenerPtr);
		_ListenerPtr = ListenerPtr;
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
        size_t AddrLen = 0;
        memset(&AddrStorage, 0, sizeof(AddrStorage));
        if (Address.IsV4()) {
            auto & Addr4 = (sockaddr_in&)AddrStorage;
            Addr4.sin_family = AF_INET;
            Addr4.sin_addr = (decltype(sockaddr_in::sin_addr)&)(Address.Ipv4);
            Addr4.sin_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in);
        } else if (Address.IsV6()) {
            auto & Addr6 = (sockaddr_in6&)AddrStorage;
            Addr6.sin6_family = AF_INET6;
            Addr6.sin6_addr = (decltype(sockaddr_in6::sin6_addr)&)(Address.Ipv6);
            Addr6.sin6_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in6);
        }

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

				Todo("SetError");
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