#pragma once

#include <xel/Common.hpp>
#include "./TcpConnectionEx.hpp"
#include <vector>

X_NS
{

	class xTcpConnectionPool
	: xTcpConnection::iListener
	, xNonCopyable
	{
	public:
		struct iListener {
            virtual void    OnConnected(xTcpConnectionPool * ConnectionPoolPtr, size_t Index) {};
            virtual size_t  OnData(xTcpConnectionPool * ConnectionPoolPtr, size_t Index, void * DataPtr, size_t DataSize) = 0;
		};

	public:
		X_API_MEMBER bool Init(xIoContext * IoContextPtr, const std::vector<xNetAddress>& Addresses, iListener * ListenerPtr);
		X_API_MEMBER bool Init(xIoContext * IoContextPtr, std::vector<xNetAddress>&& Addresses, iListener * ListenerPtr);
		X_API_MEMBER void Clean();
		X_API_MEMBER void Check(uint64_t NowMS);
		X_API_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
		X_API_MEMBER bool PostData(size_t Index, const void * DataPtr, size_t DataSize);
		X_INLINE     void SetReconnectTimeoutMS(uint64_t TimeoutMS) { _ReconnectTimeoutMS = TimeoutMS; }

	protected:
        X_API_MEMBER size_t  OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;
        X_API_MEMBER void    OnConnected(xTcpConnection * TcpConnectionPtr) override;
        X_API_MEMBER void    OnPeerClose(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); }
        X_API_MEMBER void    OnError(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); }
		X_INLINE     void    Kill(xTcpConnection * TcpConnectionPtr) { _KillConnectionList.GrabTail(static_cast<xTcpConnectionEx&>(*TcpConnectionPtr)); }

	private:
		static constexpr xTcpConnectionEx::eType DisabledConnection = 0;
		static constexpr xTcpConnectionEx::eType EnabledConnection  = 1;

	private:
		iListener *                     _ListenerPtr  = nullptr;
		xIoContext *                    _IoContextPtr = nullptr;
		xTcpConnectionEx*               _Connections  = nullptr;
		std::vector<xNetAddress>        _Addresses;

		xTcpConnectionExList            _EnabledConnectionList;
		xTcpConnectionExList            _DisabledConnectionList;
		xTcpConnectionExList            _KillConnectionList;
		uint64_t                        _ReconnectTimeoutMS = 30'000;

		ubyte KeepAliveRequestBuffer[xPacketHeader::Size];
	};

}
