#include "./UdpChannel.hpp"
#include "./NetBase.hpp"

ZEC_NS
{
    class xUdpSocketContext
    : public xRetainable
    , xNonCopyable
    {
    public:
        xUdpSocketContext(xIoContext * IoContextPtr, xUdpChannel::iListener * ListenerPtr, xUdpChannel * ListenerContextPtr)
        : _Socket(xIoCaster()(*IoContextPtr)) {
            ReceiveBuffer[MaxPacketSize] = 0;
            _Socket.open(udp::v4());
            _ListenerPtr = ListenerPtr;
            _ListenerContextPtr = ListenerContextPtr;
            DoRead();
        }
        xUdpSocketContext(xIoContext * IoContextPtr, const xNetAddress & BindAddress, xUdpChannel::iListener * ListenerPtr, xUdpChannel * ListenerContextPtr)
        : _Socket(xIoCaster()(*IoContextPtr), MakeUdpEndpoint(BindAddress)) {
            ReceiveBuffer[MaxPacketSize] = 0;
            _ListenerPtr = ListenerPtr;
            _ListenerContextPtr = ListenerContextPtr;
            DoRead();
        }
        ~xUdpSocketContext() = default;

        xUdpSocket & Native() { return _Socket; }
        const xUdpSocket & Native() const { return _Socket; }
        void Close() { assert(!CallbackFlag); _Socket.close(X2Ref(xAsioError{})); }

        void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress) {
            _Socket.send_to(xAsioConstBuffer{DataPtr, DataSize}, MakeUdpEndpoint(DestiationAddress), 0, X2Ref(xAsioError()));
        }

    protected:
        void DoRead();

    public:
        xUdpEndpoint SenderEndpoint;
        ubyte        ReceiveBuffer[MaxPacketSize + 1];
        bool         CallbackFlag = false;

    private:
        xUdpSocket               _Socket;
        xUdpChannel::iListener * _ListenerPtr = nullptr;
        xUdpChannel *            _ListenerContextPtr = nullptr;
    };

    void xUdpSocketContext::DoRead()
    {
        _Socket.async_receive_from(xAsioMutableBuffer{ReceiveBuffer, MaxPacketSize }, SenderEndpoint,
        [this, R=xRetainer{this}](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                auto ListenerPtr = Steal(_ListenerPtr);
                if (Error != asio::error::operation_aborted && ListenerPtr) {
                    ListenerPtr->OnError(Steal(_ListenerContextPtr), Error.message().c_str());
                }
                return;
            }

#ifndef NDEBUG
            CallbackFlag = true;
            _ListenerPtr->OnData(_ListenerContextPtr, ReceiveBuffer, TransferedSize, MakeNetAddress(SenderEndpoint));
            CallbackFlag = false;
#else
            _ListenerPtr->OnData(_ListenerContextPtr, ReceiveBuffer, TransferedSize, MakeNetAddress(SenderEndpoint));
#endif
            DoRead();
        });
    }

    /* user delegate */

    bool xUdpChannel::Init(xIoContext * IoContextPtr, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);

        try {
            _SocketPtr = new xUdpSocketContext(IoContextPtr, ListenerPtr, this);
        } catch (...) {
            return false;
        }
        return true;
    }

    bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);

        try {
            _SocketPtr = new xUdpSocketContext(IoContextPtr, BindAddress, ListenerPtr, this);
        } catch (...) {
            return false;
        }
        return true;
    }

    void xUdpChannel::Clean()
    {
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

    void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress)
    {
        _SocketPtr->PostData(DataPtr, DataSize, DestiationAddress);
    }

}
