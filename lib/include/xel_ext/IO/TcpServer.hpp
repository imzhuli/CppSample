#pragma once

#include <xel/Common.hpp>
#include "./IoContext.hpp"
#include "./TcpConnection.hpp"
#include "./Packet.hpp"
#include <atomic>

X_NS
{

    class xTcpServer
    : iIoReactor
	, xNonCopyable
    {
	public:
		struct iListener
		{
			virtual void OnNewConnection(xTcpServer * TcpServerPtr, xSocket NativeHandle) = 0;
		};

    public:
        X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort = false);
		X_API_MEMBER void Clean();

		X_INLINE xIoContext * GetIoContextPtr() const { return _IoContextPtr; }

	private:
		X_API_MEMBER void DoAccept();
		X_API_MEMBER void OnAccept(xSocket NativeHandle);

	private:
		xIoContext *  _IoContextPtr;
		xSocket       _ListenSocket X_DEBUG_INIT(InvalidSocket);
		iListener *   _ListenerPtr;

	#if defined(X_SYSTEM_WINDOWS)		
		ADDRESS_FAMILY    _AF;
		xSocket           _PreAcceptSocket X_DEBUG_INIT(InvalidSocket);
		struct {
			sockaddr_storage  Local;
			sockaddr_storage  Remote;
		} _PreAcceptAddress;
		DWORD             _PreAcceptReceivedLength;
        OVERLAPPED        _Overlapped;

		X_API_MEMBER void TryPreAccept();
		X_API_MEMBER void OnDeferredOperation() override { TryPreAccept(); }
		X_API_MEMBER eIoEventType GetEventType(OVERLAPPED * OverlappedPtr) {
			assert(OverlappedPtr == &_Overlapped);
			return eIoEventType::OutReady;
		};
		X_API_MEMBER void OnIoEventOutReady() override;
	#endif
	};

}
