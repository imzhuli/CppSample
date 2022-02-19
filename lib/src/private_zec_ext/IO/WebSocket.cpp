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
        _FrameBufferPtr = new xBeastDynamicBuffer;

        auto WSPtr = NativeWebSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        if (!WSPtr) {
            delete NativeBuffer(Steal(_FrameBufferPtr));
            Reset(_ListenerPtr);
            Reset(_IoContextPtr);
            return false;
        }
        _State = eInited;

        WSPtr->next_layer().async_connect(MakeAddress(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                ErrorClose();
                return;
            }
            _ListenerPtr->OnConnected(this);
            DoHandshake();
        });
        return true;
    }

    void xWebSocketClient::DoHandshake()
    {
        cout << "Do handshake" << endl;
        auto WSPtr = NativeWebSocket(Native());
        WSPtr->async_handshake(_Hostname, _Target, [this](const xAsioError & Error) {
            if (Error) {
                ErrorClose();
                return;
            }
            _ListenerPtr->OnHandshakeDone(this);
            // TODO: start waiting for reading/writing
        });
    }

    void xWebSocketClient::DoClose()
    {
        assert(_State == eClosing);
        NativeWebSocketHolderRef(Native()).Destroy();
        _State = eClosed;
    }

    void xWebSocketClient::Clean()
    {
        if (_State != eClosed) {
            assert(_State != eClosing || "Do cleanup during callback is forbidden");
            _State = eClosing;
            DoClose();
        }
        delete NativeBuffer(Steal(_FrameBufferPtr));
        Reset(_Hostname);
        Reset(_Target);
        Reset(_ListenerPtr);
        Reset(_IoContextPtr);
        _State = eUnspecified;
    }

}
