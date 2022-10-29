#pragma once

#include <xel/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./TcpConnection.hpp"
#include "./Packet.hpp"

X_NS
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
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint16_t Port, iListener * ListenerPtr, bool ReusePort = false);
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort = false);
		X_API_MEMBER void Clean();

		X_INLINE xIoContext * GetIoContextPtr() const { return _IoContextPtr; }

	private:
		X_API_MEMBER void DoAccept();
		X_API_MEMBER void OnAccept(xIoHandle NativeHandle);

	private:
		xIoContext *  _IoContextPtr;
		iListener *   _ListenerPtr;

		xDummy<16>    _Native;
        friend class __detail__::IOUtil;
	};

}
