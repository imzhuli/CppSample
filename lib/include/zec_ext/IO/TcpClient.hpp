#pragma once

#include <zec/Common.hpp>
#include <atomic>
#include "./IoContext.hpp"
#include "./PacketData.hpp"

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xTcpClient
    : xNonCopyable
    {
    public:
        static constexpr const size_t InvalidDataSize = static_cast<size_t>(-1);
        struct iListener
        {
            // callback on connected, normally this is not needed to be handled
            virtual bool   OnConnected(xTcpClient * TcpClientPtr)  { return true; }
            /***
             * OnReceivedData:
             * called when there is some data in,
             * @return consumed bytes, if return value equeals -1, something wrong happened, auto closing is involved
             * */
            virtual size_t OnReceiveData(xTcpClient * TcpClientPtr, const void * DataPtr, size_t DataSize) { return -1; }
            virtual void   OnPeerClose(xTcpClient * TcpClientPtr)  {}
            virtual void   OnError(xTcpClient * TcpClientPtr) {}
        };

    public:
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr);
        ZEC_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr);
        /***
         * @brief aync post data, try to buffer(copy) data into internal buffer
         * @return Unbuffered Data Size
         * */
        ZEC_API_MEMBER size_t PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void Clean();

    protected:
        ZEC_API_MEMBER virtual xPacketBuffer * NewWriteBuffer() { return new (std::nothrow) xPacketBuffer(); }
        ZEC_API_MEMBER virtual void DeleteWriteBuffer(xPacketBuffer * BufferPtr) { delete BufferPtr; }

    private:
        ZEC_INLINE void * Native() { return (void*)_Dummy; }
        ZEC_PRIVATE_MEMBER void DoRead();
        ZEC_PRIVATE_MEMBER void DoWrite();

        ZEC_INLINE void ErrorClose() { if (_State >= eClosing) return; _ListenerPtr->OnError(this); DoClose(); }
        ZEC_PRIVATE_MEMBER void DoClose(); // cleanup socket, but not the object

    private:
        xIoContext *                  _IoContextPtr {};
        xNetAddress                   _ServerAddress {};
        uint16_t                      _ServerPort {};
        iListener *                   _ListenerPtr = nullptr;
        xPacketBufferQueue            _WritePacketBufferQueue;

        ubyte                         _ReadBuffer[MaxPacketSize];
        size_t                        _ReadDataSize = 0;
        size_t                        _WriteDataSize = 0;

        enum : uint8_t {
            eUnspecified, eInited, eConnected, eClosing, eClosed
        } _State = eUnspecified;

        alignas(max_align_t) ubyte    _Dummy[64];
        friend class __detail__::IOUtil;
    };

}
