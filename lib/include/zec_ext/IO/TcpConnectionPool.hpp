#pragma once

#include <zec/Common.hpp>
#include "./TcpConnectionEx.hpp"
#include <vector>

ZEC_NS
{

	class xTcpConnectionPool
	: xTcpConnection::iListener
	, xNonCopyable
	{
	public:
		struct iListener {
            virtual size_t  OnData(xTcpConnectionPool * ConnectionPoolPtr, void * DataPtr, size_t DataSize) = 0;
		};

	public:
		ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, std::vector<xNetAddress>&& Addresses, iListener * ListenerPtr);
		ZEC_API_MEMBER void Clean();
		ZEC_API_MEMBER void Check(uint64_t NowMS);
		ZEC_API_MEMBER void PostData(const void * DataPtr, size_t DataSize);
		ZEC_INLINE     void SetReconnectTimeoutMS(uint64_t TimeoutMS) { _ReconnectTimeoutMS = TimeoutMS; }

	protected:
        ZEC_API_MEMBER size_t  OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;
        ZEC_API_MEMBER void    OnConnected(xTcpConnection * TcpConnectionPtr) override;
        ZEC_API_MEMBER void    OnPeerClose(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); }
        ZEC_API_MEMBER void    OnError(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); }
		ZEC_INLINE     void    Kill(xTcpConnection * TcpConnectionPtr) { _KillConnectionList.GrabTail(static_cast<xTcpConnectionEx&>(*TcpConnectionPtr)); }

	private:
		iListener *                     _ListenerPtr  = nullptr;
		xIoContext *                    _IoContextPtr = nullptr;
		xTcpConnectionEx*               _Connections  = nullptr;
		std::vector<xNetAddress>        _Addresses;

		xTcpConnectionExList            _EnabledConnectionList;
		xTcpConnectionExList            _DisabledConnectionList;
		xTcpConnectionExList            _KillConnectionList;
		uint64_t                        _ReconnectTimeoutMS = 30'000;
	};

}
