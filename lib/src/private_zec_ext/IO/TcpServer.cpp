// #include <zec_ext/IO/TcpServer.hpp>
// #include "./_Local.hpp"

// ZEC_NS
// {

// 	static constexpr const size_t AcceptorSize = sizeof(xNativeTcpAcceptor);

// 	bool xTcpServer::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
// 	{
//         xNetAddress Address = xNetAddress::Make(Ip);
// 		return Init(IoContextPtr, Address, Port, ListenerPtr);
// 	}

// 	bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr)
// 	{
// 		_IoContextPtr = IoContextPtr;
// 		_ListenerPtr = ListenerPtr;

// 		auto & Acceptor = _Native.CreateValueAs<xNativeTcpAcceptor>(*IOUtil::Native(IoContextPtr), MakeEndpoint(Address, Port));
// 		xNativeTcpSocket::reuse_address Option(true);
// 		Acceptor.set_option(Option);
// 		DoAccept();
// 		return true;
// 	}

// 	void xTcpServer::Clean()
// 	{
// 		_Native.DestroyAs<xNativeTcpAcceptor>();
// 		_ListenerPtr = nullptr;
// 		_IoContextPtr = nullptr;
// 	}

// 	void xTcpServer::DoAccept()
// 	{
// 		auto & Acceptor = _Native.As<xNativeTcpAcceptor>();
// 		Acceptor.async_accept(*IOUtil::Native(_IoContextPtr), [this](const xAsioError & Error, xNativeTcpSocket Peer) {
// 			if (!Error) {
// 				OnAccept(xIoHandle{&Peer});
// 			}
// 			DoAccept();
// 		});
// 		return;
// 	}

// 	void xTcpServer::OnAccept(xIoHandle NativeHandle)
// 	{
// 		auto NewConnectionPtr = _ListenerPtr->OnNewConnection(_IoContextPtr, NativeHandle);
// 		NewConnectionPtr->OnConnected();
// 	}

// }
