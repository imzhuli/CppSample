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

    class xTcpServer
    : xNonCopyable
    {
	public:
		struct iListener
		{
			virtual xTcpConnection * OnNewConnection(xTcpConnection::xNativeHandle NativeHandle) = 0;
		};

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr);
		ZEC_API_MEMBER void Clean();

	private:
		ZEC_API_MEMBER void DoAccept();
		ZEC_API_MEMBER void OnAccept(xTcpConnection::xNativeHandle NativeHandle);

	private:
		xIoContext *  _IoContextPtr;
		iListener *   _ListenerPtr;

		xDummy<80>                _Native;
        friend class __detail__::IOUtil;
	};

}
