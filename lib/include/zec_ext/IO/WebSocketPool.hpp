#pragma once
#include "./WebSocketEx.hpp"

ZEC_NS
{

	class xWebSocketSessionPool
	: public xWebSocketSession::iListener
	, public xNonCopyable
    {
	public:
		struct iListener {
            virtual void OnWSMessage(xWebSocketSessionPool * WebSocketPoolPtr, bool Binary, void * DataPtr, size_t DataSize) {}
		};

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress * AddressList, size_t AddressCount, const std::string & Origin, const std::string &Target, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();
		ZEC_API_MEMBER void Check(uint64_t NowMS);

        ZEC_API_MEMBER bool PostTextData(const std::string_view & Data);
        ZEC_API_MEMBER bool PostBinaryData(const void * DataPtr, size_t DataSize);


        ZEC_API_MEMBER void OnWSMessage(xWebSocketSession * WebSocketClientPtr, bool Binary, void * DataPtr, size_t DataSize) override;
        ZEC_API_MEMBER void OnWSClose(xWebSocketSession * WebSocketClientPtr) override;

    private:
		ZEC_INLINE void Kill(xWebSocketSession * SessionPtr) { _KillSessionList.GrabTail(*static_cast<xWebSocketSessionEx*>(SessionPtr)); }

        struct xSessionTarget
        {
            xNetAddress            Address;
            xWebSocketSessionEx *  SessionPtr = nullptr;
        };

		iListener *                         _ListenerPtr  = nullptr;
		xIoContext *                        _IoContextPtr = nullptr;
        std::string                         _Origin;
        std::string                         _Target;

        std::vector<xSessionTarget>         _SessionTargets;
		xWebSocketSessionList               _EnabledSessionList;
		xWebSocketSessionList               _DisabledSessionList;
		xWebSocketSessionList               _KillSessionList;
		uint64_t                            _ReconnectTimeoutMS = 5'000;
    };

}
