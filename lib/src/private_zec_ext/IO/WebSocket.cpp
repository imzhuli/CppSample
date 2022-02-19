#include <zec_ext/IO/WebSocket.hpp>
#include <array>
#include <iostream>
#include "./_Local.hpp"

using namespace std;

ZEC_NS
{

    static inline xBeastDynamicBuffer * NativeBuffer(void * BufferPtr) { return (xBeastDynamicBuffer*)BufferPtr; }

    bool xWebSocketClient::Init(xIoContext * IoContextPtr, const char * IpStr, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr)
    {
        xNetAddress Address = xNetAddress::Make(IpStr);
        return Init(IoContextPtr, Address, Port, Hostname, Target, ListenerPtr);
    }

    bool xWebSocketClient::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, const std::string & Hostname, const std::string &Target, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(Address);
        assert(Port);

        assert(!_IoContextPtr);
        assert(!_ListenerPtr);

        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _Hostname = Hostname;
        _Target = Target;
        _ReadBufferPtr = new xBeastDynamicBuffer;

        auto WSPtr = NativeWebSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        if (!WSPtr) {
            delete NativeBuffer(Steal(_ReadBufferPtr));
            Reset(_ListenerPtr);
            Reset(_IoContextPtr);
            return false;
        }
        _State = eInited;

        WSPtr->next_layer().async_connect(MakeAddress(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            _ListenerPtr->OnConnected(this);
            DoHandshake();
        });
        return true;
    }

    void xWebSocketClient::DoHandshake()
    {
        auto WSPtr = NativeWebSocket(Native());
        WSPtr->async_handshake(_Hostname, _Target, [this](const xAsioError & Error) {
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            _ListenerPtr->OnHandshakeDone(this);
            _State = eConnected;
            if (!_MessageBufferList.IsEmpty()) {
                DoFlush();
            }
            DoRead();
        });
    }

    void xWebSocketClient::DoRead()
    {
        auto WSPtr = NativeWebSocket(Native());
        auto ReadBufferPtr = NativeBuffer(_ReadBufferPtr);
        WSPtr->async_read(*ReadBufferPtr, [this, WSPtr, ReadBufferPtr](const xAsioError & Error, size_t TransferedSize){
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            std::string Data;
            auto BufferSeq = ReadBufferPtr->data();
            for (auto Buffer : BufferSeq) {
                Data.append((const char *)Buffer.data(), Buffer.size());
            }
            _ListenerPtr->OnMessage(this, WSPtr->got_binary(), Data.data(), Data.size());
            ReadBufferPtr->clear();
            DoRead();
        });
    }

    void xWebSocketClient::DoFlush()
    {
        auto WSPtr = NativeWebSocket(Native());
        auto MessagePtr = _MessageBufferList.Head();
        if (!MessagePtr) {
            return;
        }
        auto & MessageData = MessagePtr->Data;

        WSPtr->binary(MessagePtr->Binary);
        WSPtr->async_write(xAsioConstBuffer(MessageData.data(), MessageData.size()), [this, MessagePtr](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            cout << "SendData: " << MessagePtr->Data << endl;
            DeleteMessageBuffer(MessagePtr);
            if (_MessageBufferList.IsEmpty()) {
                return;
            }
            DoFlush();
        });
    }

    bool xWebSocketClient::PostData(const std::string_view & DataView, bool Binary)
    {
        assert(_State != eShuttingDown);
        bool ExecWriteCall = (_MessageBufferList.IsEmpty() && _State == eConnected);
        auto BufferPtr = NewMessageBuffer();
        if (!BufferPtr) {
            return false;
        }
        BufferPtr->Data = DataView;
        BufferPtr->Binary = Binary;
        _MessageBufferList.AddTail(*BufferPtr);
        if (ExecWriteCall) {
            DoFlush();
        }
        return true;
    }

    void xWebSocketClient::Clean()
    {
        NativeWebSocketHolderRef(Native()).Destroy();
        for (auto & MessageBuffer : _MessageBufferList) {
            DeleteMessageBuffer(&MessageBuffer);
        }
        delete NativeBuffer(Steal(_ReadBufferPtr));
        Reset(_Hostname);
        Reset(_Target);
        Reset(_ListenerPtr);
        Reset(_IoContextPtr);
        _State = eUnspecified;
    }

}
