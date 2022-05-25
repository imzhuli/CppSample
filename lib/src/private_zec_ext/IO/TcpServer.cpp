#include "./_Local.hpp"
#include "./NetBase.hpp"
#include "./TcpConnection.hpp"
#include <zec_ext/IO/TcpServer.hpp>


#if defined(ZEC_SYSTEM_LINUX) || defined(ZEC_SYSTEM_ANDROID)
#define ZEC_ENABLE_REUSEPORT SO_REUSEPORT
#include <sys/socket.h>
#if defined(SO_REUSEPORT_LB)
#undef  ZEC_ENABLE_REUSEPORT
#define ZEC_ENABLE_REUSEPORT SO_REUSEPORT_LB
#endif
#endif

ZEC_NS
{
    using xNativeTcpAcceptor = tcp::acceptor;
    using xSharedTcpAcceptorPtr = std::shared_ptr<xNativeTcpAcceptor>;

	bool xTcpServer::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
	{
        xNetAddress Address = xNetAddress::Make(Ip, Port);
		return Init(IoContextPtr, Address, ListenerPtr);
	}

	bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
	{
		try {
			auto & Acceptor = _Native.CreateValueAs<xSharedTcpAcceptorPtr>(new xNativeTcpAcceptor(xIoCaster()(*IoContextPtr)));
			Acceptor->open(tcp::v4());
			xTcpSocket::reuse_address Option(true);
			Acceptor->set_option(Option);
		#ifdef ZEC_ENABLE_REUSEPORT
			int one = 1;
    		setsockopt(Acceptor->native_handle(), SOL_SOCKET, ZEC_ENABLE_REUSEPORT, &one, sizeof(one));
		#endif
			Acceptor->bind(MakeTcpEndpoint(Address));
			Acceptor->listen();
		}
		catch (...) {
			_Native.DestroyAs<xSharedTcpAcceptorPtr>();
			return false;
		}

		_IoContextPtr = IoContextPtr;
		_ListenerPtr = ListenerPtr;
		DoAccept();
		return true;
	}

	void xTcpServer::Clean()
	{
		_Native.DestroyAs<xSharedTcpAcceptorPtr>();
		_ListenerPtr = nullptr;
		_IoContextPtr = nullptr;
	}

	void xTcpServer::DoAccept()
	{
		auto & Acceptor = _Native.As<xSharedTcpAcceptorPtr>();
		Acceptor->async_accept(xIoCaster()(*_IoContextPtr), [this, R=Acceptor](const xAsioError & Error, xTcpSocket Peer) {
			if (!Error) {
				OnAccept(xIoHandle{&Peer});
			}
			DoAccept();
		});
		return;
	}

	void xTcpServer::OnAccept(xIoHandle NativeHandle)
	{
		_ListenerPtr->OnNewConnection(this, NativeHandle);
	}

}
