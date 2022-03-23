#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./TcpConnection.hpp"
#include "./Packet.hpp"

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
			virtual void OnNewConnection(xTcpServer * TcpServerPtr, xIoHandle NativeHandle) = 0;
		};

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
		ZEC_API_MEMBER void Clean();

		ZEC_INLINE xIoContext * GetIoContextPtr() const { return _IoContextPtr; }

	private:
		ZEC_API_MEMBER void DoAccept();
		ZEC_API_MEMBER void OnAccept(xIoHandle NativeHandle);

	private:
		xIoContext *  _IoContextPtr;
		iListener *   _ListenerPtr;

		xDummy<16>    _Native;
        friend class __detail__::IOUtil;
	};

}
