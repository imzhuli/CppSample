#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./TcpConnection.hpp"
#include "./PacketData.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }
    class xTcpServer;

    class xTcpServer final
    : xNonCopyable
    {
    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, xTcpConnection::iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, xTcpConnection::iListener * ListenerPtr);
		ZEC_API_MEMBER void Clean();

		ZEC_API_MEMBER void RecycleConnection(xTcpConnection *);

	private:
		ZEC_API_MEMBER xTcpConnection * CreateConnection();
		ZEC_API_MEMBER void DoAccept();
		ZEC_API_MEMBER void OnAccept(xTcpConnection * ConnectionPtr);
		ZEC_API_MEMBER void OnError(xTcpConnection * ConnectionPtr);

	private:
		xIoContext *                  _IoContextPtr;
		xTcpConnection::iListener *   _ClientListenerPtr;

		bool                          _Error = false;

		xDummy<80>                _Native;
        friend class __detail__::IOUtil;
	};

}
