#include <zec_ext/IO/WebSocket.hpp>
#include <array>
#include <iostream>
#include <memory>
#include "./_Local.hpp"

using namespace std;

ZEC_NS
{

    using xWsPtr = std::shared_ptr<xNativeWebSocket>;
    using xWSReadBufferPtr = std::shared_ptr<xBeastDynamicBuffer>;

    xWebSocketSession::xWebSocketSession()
    {
        _ReadBuffer.CreateAs<xWSReadBufferPtr>();
        _Native.CreateAs<xWsPtr>();
    }
    xWebSocketSession::~xWebSocketSession()
    {
        Clean();
        _Native.DestroyAs<xWsPtr>();
        _ReadBuffer.DestroyAs<xWSReadBufferPtr>();
    }

    bool xWebSocketSession::Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr)
    {
        _Origin = Hostname;
        _Path = Target;
        _Listener = ListenerPtr;
        _ReadBuffer.As<xWSReadBufferPtr>() = std::make_shared<xBeastDynamicBuffer>();

        auto & WS = _Native.As<xWsPtr>();
        WS = std::make_shared<xNativeWebSocket>(*IOUtil::Native(IoContextPtr));
        WS->next_layer().async_connect(ip::tcp::endpoint(ip::make_address(IpStr), Port), [this, Retainer=WS](xAsioError Error) mutable {
            if (Error) {
                // cerr << "WS Connect Error" << endl;
                OnError(Retainer.get());
                return;
            }
            OnConnected(Retainer.get());
        });
        return true;
    }

    void xWebSocketSession::Clean()
    {
        auto & WS = _Native.As<xWsPtr>();
        if (WS) {
            WS->next_layer().close(X2Ref(xAsioError{}));
            WS.reset();
        }
        _ReadBuffer.As<xWSReadBufferPtr>().reset();
        for(auto & Message : _MessageBufferList) {
            delete &Message;
        }
        _Connected = false;
        _Error = false;
    }

    bool xWebSocketSession::OnError(const void * CallbackObjectPtr)
    {
        auto & WS = _Native.As<xWsPtr>();
        if (WS.get() != CallbackObjectPtr) {
            // cerr << "OnError: Callback from abandoned object" << endl;
            return false;
        }
        if (!_Error) {
            _Error = true;
            _Connected = false;
            _Listener->OnError(this);
        }
        return true;
    }

    void xWebSocketSession::OnConnected(const void * CallbackObjectPtr) {
        auto & WS = _Native.As<xWsPtr>();
        if (WS.get() != CallbackObjectPtr) {
            // cerr << "OnConnected: Callback from abandoned object" << endl;
            return;
        }
        WS->async_handshake(_Origin, _Path, [this, Retainer=WS] (const xAsioError & Error) mutable {
            if (Error) {
                // cerr << "Handshake Error" << endl;
                OnError(Retainer.get());
                return;
            }
            OnHandshakeDone(Retainer.get());
        });
    }

    void xWebSocketSession::OnHandshakeDone(const void * CallbackObjectPtr) {
        auto & WS = _Native.As<xWsPtr>();
        if (WS.get() != CallbackObjectPtr) {
            // cerr << "OnConnected: Callback from abandoned object" << endl;
            return;
        }
        _Listener->OnHandshakeDone(this);
        _Connected = true;
        DoRead();
        DoFlush();
    }

    void xWebSocketSession::DoRead()
    {
        auto & WS = _Native.As<xWsPtr>();
        auto & RB = _ReadBuffer.As<xWSReadBufferPtr>();
        WS->async_read(*RB, [this, ReadBufferPtr = RB, Retainer=WS](const xAsioError & Error, size_t TransferedSize){
            if (Error) {
                // cerr << "WS Read Error" << endl;
                OnError(Retainer.get());
                return;
            }
            std::string Data;
            auto BufferSeq = ReadBufferPtr->data();
            for (auto Buffer : BufferSeq) {
                Data.append((const char *)Buffer.data(), Buffer.size());
            }
            _Listener->OnMessage(this, Retainer->got_binary(), Data.data(), Data.size());
            ReadBufferPtr->clear();
            DoRead();
        });
    }

    void xWebSocketSession::DoFlush()
    {
        auto MessagePtr = _MessageBufferList.Head();
        if (!MessagePtr) {
            return;
        }
        auto & MessageData = MessagePtr->Data;
        auto & WS = _Native.As<xWsPtr>();
        WS->binary(MessagePtr->Binary);
        WS->async_write(xAsioConstBuffer(MessageData.data(), MessageData.size()), [this, Retainer=WS, MessagePtr](const xAsioError & Error, size_t TransferedSize) {
            do {
                auto MessageCleaner = xScopeGuard{ [MessagePtr] { delete MessagePtr; } };
                if (Error) {
                    // cerr << "WS Flush Error" << endl;
                    OnError(Retainer.get());
                    return;
                }
            } while(false);
            DoFlush();
        });
    }

    bool xWebSocketSession::PostTextData(const std::string_view & Data)
    {
        xMessageBuffer * BufferPtr = new xMessageBuffer;
        BufferPtr->Data = Data;
        return DoPostMessage(BufferPtr);
    }

    bool xWebSocketSession::PostBinaryData(const void * DataPtr, size_t DataSize)
    {
        xMessageBuffer * BufferPtr = new xMessageBuffer;
        BufferPtr->Binary = true;
        BufferPtr->Data = std::string((const char *)DataPtr,  DataSize);
        return DoPostMessage(BufferPtr);
    }

    bool xWebSocketSession::DoPostMessage(xMessageBuffer * MessagePtr)
    {
        bool ExecWriteCall = (_MessageBufferList.IsEmpty() && _Connected);
        _MessageBufferList.AddTail(*MessagePtr);
        if (ExecWriteCall) {
            DoFlush();
        }
        return true;
    }

}
