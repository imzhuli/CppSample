#include <xel_ext/IO/UdpChannel.hpp>

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)

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
        if (BindRet == -1) {
            return false;
        }

#if defined(X_SYSTEM_LINUX)
        struct epoll_event Event = {};
        Event.data.ptr = this;
        Event.events = EPOLLET | EPOLLIN;
        if (-1 == epoll_ctl(*IoContextPtr, EPOLL_CTL_ADD, _Socket, &Event)) {
            X_DEBUG_PRINTF("xUdpChannel::Init failed to register epoll event\n");
            return false;
        }
#else /* defined(X_SYSTEM_DARWIN) */
        struct kevent Event[1] = {};
        Event[0].ident = _Socket;
        Event[0].flags = EV_ADD | EV_CLEAR;
        Event[0].filter = EVFILT_READ;
        Event[0].udata = this;
        if (-1 == kevent(*IoContextPtr, Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("xUdpChannel::Init failed to register kevent\n");
            return false;
        }
#endif
		_IoContextPtr = IoContextPtr;
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
		auto SendResult = sendto(_Socket, (const char *)DataPtr, DataSize, 0, (const sockaddr*)&AddrStorage, (socklen_t)AddrLen);
        if (SendResult == -1) {
            [[maybe_unused]] auto Error = errno;
            X_DEBUG_PRINTF("Udp send error: code=%i, description=%s\n", Error, strerror(Error));
        }
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