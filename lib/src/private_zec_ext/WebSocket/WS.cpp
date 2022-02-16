#include <zec_ext/WebSocket/WS.hpp>
#include <libwebsockets.h>
#include <mutex>

ZEC_NS
{
    static constexpr const size_t LWSMaxFrameSize = 1024*1024;

    struct xWebSocketSession
    {
        lws * ConnectionPtr = nullptr;
        xWebSocketClient * ClientPtr = nullptr;
        unsigned char Buffer[LWS_PRE + LWSMaxFrameSize];
    };

    namespace __detail__ {

        class xWebSocketWrapper{
        public:
            static int LWSCallback(lws * WSIPtr, enum lws_callback_reasons Reason, void * UserCtxPtr, void * InPtr, size_t InLength)
            {
                auto SessionPtr = (xWebSocketSession*)UserCtxPtr;
                auto WSClientPtr = SessionPtr ? SessionPtr->ClientPtr : nullptr;
                switch (Reason) {
                    case LWS_CALLBACK_CLIENT_RECEIVE: {
                        if (!WSClientPtr || !WSClientPtr->OnWSDataIn(InPtr, InLength)) {
                            return -1;
                        }
                        break;
                    }
                    case LWS_CALLBACK_CLIENT_WRITEABLE: {
                        if (!WSClientPtr) {
                            return -1;
                        }
                        for (auto & Message: WSClientPtr->_RelayMessages) {
                            auto DataStartPtr = SessionPtr->Buffer + LWS_PRE;
                            memcpy(DataStartPtr, (unsigned char*)Message.data(), Message.length());
                            lws_write(WSIPtr, DataStartPtr, Message.length(), LWS_WRITE_TEXT);
                        }
                        WSClientPtr->_RelayMessages.clear();
                        break;
                    }
                    case LWS_CALLBACK_WSI_DESTROY: {
                        if (!WSClientPtr) {
                            delete SessionPtr;
                            break;
                        }
                        delete Steal(WSClientPtr->_WSSocketSessionPtr);
                        WSClientPtr->OnWSDisconnected();
                        return 0;
                    }
                    default:  {
                        break;
                    }
                }
                return 0;
            }
        };
    }

    static lws_protocols WSProtocols[] = {
        { "ws", __detail__::xWebSocketWrapper::LWSCallback, 0, LWSMaxFrameSize },
        { nullptr, nullptr, 0 }
    };

    static std::once_flag DisableLogFlag;
    static void IgnoreLWSLogging(int level, const char *line) {}

    bool xWebSocketContext::Init()
    {
        std::call_once(DisableLogFlag, []{ lws_set_log_level(LLL_ERR, &IgnoreLWSLogging); });

        lws_context_creation_info WSCreateInfo = {};
        WSCreateInfo.port = CONTEXT_PORT_NO_LISTEN;
        WSCreateInfo.protocols = WSProtocols;
        WSCreateInfo.gid = -1;
        WSCreateInfo.uid = -1;
        WSCreateInfo.ka_time = 60;
        WSCreateInfo.ka_probes = 1;
        WSCreateInfo.ka_interval = 3;
        return (_WSContextPtr.Ptr = lws_create_context(&WSCreateInfo));
    }

    void xWebSocketContext::LoopOnce(int TimeoutMS)
    {
        lws_service((lws_context*)_WSContextPtr.Ptr, TimeoutMS);
    }

    void xWebSocketContext::Clean()
    {
        assert(_WSContextPtr.Ptr);
        lws_context_destroy((lws_context*)Steal(_WSContextPtr.Ptr));
    }

    bool xWebSocketClient::Init(xWebSocketContext * WSContextPtr, const xConfig & Config)
    {
        auto SessionPtr = new xWebSocketSession;

        lws_client_connect_info WSConnectionInfo = {};
        WSConnectionInfo.context = (lws_context*)WSContextPtr->_WSContextPtr.Ptr;
        WSConnectionInfo.address = Config.Hostname.c_str();
        WSConnectionInfo.port = Config.Port;
        WSConnectionInfo.path = Config.Path.c_str();
        WSConnectionInfo.host = Config.Hostname.c_str();
        WSConnectionInfo.protocol = "ws";
        WSConnectionInfo.userdata = SessionPtr;

        // origin:
        std::string Origin = Config.Origin;
        if (Origin.empty()) {
            Origin = Config.Hostname + ":" + std::to_string(Config.Port);
        }
        WSConnectionInfo.origin = Origin.c_str();

        if ((SessionPtr->ConnectionPtr = lws_client_connect_via_info(&WSConnectionInfo))) {
            SessionPtr->ClientPtr = this;
            _WSSocketSessionPtr = SessionPtr;
            return true;
        }
        delete SessionPtr;
        return false;
    }

    void xWebSocketClient::Post(const void * DataPtr, size_t Size)
    {
        if (!IsConnected()) {
            return;
        }
        _RelayMessages.emplace_back((const char*)DataPtr, Size);
        lws_callback_on_writable(_WSSocketSessionPtr->ConnectionPtr);
    }

    void xWebSocketClient::Clean()
    {
        if (auto SessionPtr = Steal(_WSSocketSessionPtr)) {
            SessionPtr->ClientPtr = nullptr;
            lws_callback_on_writable(SessionPtr->ConnectionPtr);
        }
    }

}
