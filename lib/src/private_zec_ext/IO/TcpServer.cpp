#include <zec_ext/IO/TcpServer.hpp>
#include "./_Local.hpp"

ZEC_NS
{

	static constexpr const size_t AcceptorSize = sizeof(xNativeTcpAcceptor);

	bool xTcpServer::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, xTcpConnection::iListener * ListenerPtr)
	{
        xNetAddress Address = xNetAddress::Make(Ip);
		return Init(IoContextPtr, Address, Port, ListenerPtr);
	}

	bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, xTcpConnection::iListener * ListenerPtr)
	{
		_IoContextPtr = IoContextPtr;
		_ClientListenerPtr = ListenerPtr;

		auto & Acceptor = _Native.CreateValueAs<xNativeTcpAcceptor>(*IOUtil::Native(IoContextPtr), MakeEndpoint(Address, Port));
		xNativeTcpSocket::reuse_address Option(true);
		Acceptor.set_option(Option);
		DoAccept();
		return true;
	}

	void xTcpServer::Clean()
	{
		_Native.DestroyAs<xNativeTcpAcceptor>();
		_ClientListenerPtr = nullptr;
		_IoContextPtr = nullptr;
		_Error = false;
	}

	xTcpConnection * xTcpServer::CreateConnection()
	{
		auto ConnectionPtr = new xTcpConnection;
		ConnectionPtr->Init(_IoContextPtr, _ClientListenerPtr);
		return ConnectionPtr;
	}

	void xTcpServer::RecycleConnection(xTcpConnection * ConnectionPtr)
	{
		ConnectionPtr->Clean();
		delete ConnectionPtr;
	}

	void xTcpServer::DoAccept()
	{
		auto & Acceptor = _Native.As<xNativeTcpAcceptor>();
		auto ConnectionPtr = CreateConnection();
		Acceptor.async_accept(*IOUtil::Native(ConnectionPtr), [this, ConnectionPtr](const xAsioError & Error) {
			if (Error) {
				OnError(ConnectionPtr);
				// do NOT return here,
				// just try next acception
			} else {
				OnAccept(ConnectionPtr);
			}
			DoAccept();
		});
		return;
	}

	void xTcpServer::OnAccept(xTcpConnection * ConnectionPtr)
	{
		ConnectionPtr->OnConnected();
	}

	void xTcpServer::OnError(xTcpConnection * ConnectionPtr)
	{
		RecycleConnection(ConnectionPtr);
	}

}
