#include "./_Local.hpp"
#include "./NetBase.hpp"
#include "./TcpConnection.hpp"
#include <zec_ext/IO/TcpServer.hpp>

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
		_IoContextPtr = IoContextPtr;
		_ListenerPtr = ListenerPtr;

		auto & Acceptor = _Native.CreateValueAs<xSharedTcpAcceptorPtr>(new xNativeTcpAcceptor(xIoCaster()(*IoContextPtr), MakeTcpEndpoint(Address)));
		xTcpSocket::reuse_address Option(true);
		Acceptor->set_option(Option);
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
