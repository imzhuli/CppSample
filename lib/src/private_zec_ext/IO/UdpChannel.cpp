#include "./UdpChannel.hpp"
#include "./NetBase.hpp"

ZEC_NS
{
    class xUdpSocketContext
    : public xRetainable
    , xNonCopyable
    {
    public:
        xUdpSocketContext(xIoContext * IoContextPtr)
        : _Socket(xIoCaster()(*IoContextPtr)) {
            ReceiveBuffer[MaxPacketSize] = 0;
        }
        xUdpSocketContext(xIoContext * IoContextPtr, const xNetAddress & BindAddress)
        : _Socket(xIoCaster()(*IoContextPtr), MakeUdpEndpoint(BindAddress)) {
            ReceiveBuffer[MaxPacketSize] = 0;
        }
        ~xUdpSocketContext() = default;

        xUdpSocket & Native() { return _Socket; }
        const xUdpSocket & Native() const { return _Socket; }
        void Close() { assert(!CallbackFlag); _Socket.close(X2Ref(xAsioError{})); }
    public:
        xUdpEndpoint SenderEndpoint;
        ubyte        ReceiveBuffer[MaxPacketSize + 1];
        bool         CallbackFlag = false;

    private:
        xUdpSocket _Socket;
    };

    bool xUdpChannel::Init(xIoContext * IoContextPtr, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);

        _SocketPtr = new xUdpSocketContext(IoContextPtr);
        _SocketPtr->Native().open(udp::v4());
        if (!_SocketPtr->Native().is_open()) {
            delete _SocketPtr;
            return false;
        }

        _ListenerPtr = ListenerPtr;
        DoRead();
        return true;
    }

    bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);

        _SocketPtr = new xUdpSocketContext(IoContextPtr, BindAddress);
        _ListenerPtr = ListenerPtr;
        DoRead();
        return true;
    }

    void xUdpChannel::Clean()
    {
        _ListenerPtr = nullptr;
        auto SocketPtr = xRetainer{ NoRetain, Steal(_SocketPtr) };
        SocketPtr->Close();
    }

    void xUdpChannel::ResizeSendBuffer(size_t Size)
    {
        asio::socket_base::send_buffer_size Option((int)Size);
        _SocketPtr->Native().set_option(Option);
    }

    void xUdpChannel::ResizeReceiveBuffer(size_t Size)
    {
        asio::socket_base::receive_buffer_size Option((int)Size);
        _SocketPtr->Native().set_option(Option);
    }

    void xUdpChannel::DoRead()
    {
        _SocketPtr->Native().async_receive_from(xAsioMutableBuffer{_SocketPtr->ReceiveBuffer, MaxPacketSize }, _SocketPtr->SenderEndpoint,
        [this, R=xRetainer{_SocketPtr}](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                auto ListenerPtr = Steal(_ListenerPtr);
                if (ListenerPtr) {
                    ListenerPtr->OnError(this, Error.message().c_str());
                }
                return;
            }
#ifndef NDEBUG
            _SocketPtr->CallbackFlag = true;
            _ListenerPtr->OnData(this, _SocketPtr->ReceiveBuffer, TransferedSize, MakeNetAddress(_SocketPtr->SenderEndpoint));
            _SocketPtr->CallbackFlag = false;
#else
            _ListenerPtr->OnData(this, _SocketPtr->ReceiveBuffer, TransferedSize, MakeNetAddress(_SocketPtr->SenderEndpoint));
#endif
            DoRead();
        });
    }

    void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress)
    {
        _SocketPtr->Native().send_to(xAsioConstBuffer{DataPtr, DataSize}, MakeUdpEndpoint(DestiationAddress), 0, X2Ref(xAsioError()));
    }

}
