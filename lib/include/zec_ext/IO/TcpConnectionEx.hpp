#pragma once

#include <zec/Common.hpp>
#include <zec/Util/MemoryPool.hpp>
#include <zec/Util/IndexedStorage.hpp>
#include "./TcpConnection.hpp"

ZEC_NS
{

    struct xTcpConnectionExNode : xListNode
    {
        using eType = uint8_t;
        static constexpr const eType eUnSpecifiedType = 0;

        eType    ConnectionType = eUnSpecifiedType;
        uint64_t ConnectionId  = 0;
        uint64_t ConnectionTimestampMS   = 0;

        using xListNode::Detach;
    };
    using xTcpConnectionExList = xList<xTcpConnectionExNode>;

    struct xTcpConnectionEx
    : xTcpConnection
    , xTcpConnectionExNode
    {};

    class xAutoClientTcpConnection
    : xTcpConnection::iListener
    {
    public:
        struct iListener {
            virtual size_t OnData(xAutoClientTcpConnection * ConnectionPtr, void * DataPtr, size_t DataSize) = 0;
        };

        ZEC_API_MEMBER bool Init(xIoContext * ContextPtr, const xNetAddress ServerAddress, iListener * ListenerPtr);
        ZEC_API_MEMBER void Clean();

        ZEC_API_MEMBER void        Check(uint64_t NowMS);
        ZEC_API_MEMBER void        ResetConnection() { _Dying = true; }
        ZEC_API_MEMBER size_t      PostData(const void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER bool        IsConnected() const { return _Connected; }
        ZEC_API_MEMBER uint64_t    GetVersion() const { return _Version; }

    protected:
        ZEC_API_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr);
        ZEC_API_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize);
        ZEC_API_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr);
        ZEC_API_MEMBER void   OnError(xTcpConnection * TcpConnectionPtr);

    private:
        xNetAddress                  _ServerAddress;
        xTcpConnection               _Connection;
        xIoContext *                 _IoContextPtr = nullptr;
        iListener *                  _ListenerPtr = nullptr;
        uint64_t                     _CheckTimestamp = 0;
        uint64_t                     _Version = 0;
        bool                         _Active = false;
        bool                         _Connected = false;
        bool                         _Dying = false;
    };

    template<typename T = xTcpConnectionEx>
    class xTcpConnectionManager
    {
        static_assert(std::is_base_of_v<xTcpConnectionEx, T>);
    public:
        bool Init(size_t PoolSize)
        {
            xMemoryPoolOptions PoolOptions = {
                .InitSize          = 1'0000,
                .Addend            = 1'0000,
                .MaxSizeIncrement  = std::max(size_t(1'0000), PoolSize / 10),
            };
            PoolOptions.MaxPoolSize = PoolSize;
                if (!Pool.Init(PoolOptions)) {
                    return false;
                }
            if (!KeyManager.Init(PoolSize)) {
                Pool.Clean();
                return false;
            }
            return true;
        }
        void Clean() {
            KeyManager.Clean();
            Pool.Clean();
        }

        ZEC_INLINE T* GetConnectionById(xIndexId Key) {
            auto Opt = KeyManager.CheckAndGet(Key);
            return Opt() ? *Opt : nullptr;
        }

        ZEC_INLINE T* CreateConnection() {
            auto ConnectionPtr = Pool.Create();
            if (!ConnectionPtr) {
                return nullptr;
            }
            ConnectionPtr->ConnectionId = KeyManager.Acquire(ConnectionPtr);
            return ConnectionPtr;
        }

        ZEC_INLINE void DestroyConnection(T* ConnectionPtr) {
            KeyManager.Release(Steal(ConnectionPtr->ConnectionId));
            Pool.Destroy(ConnectionPtr);
        }

    private:
        xIndexedStorage<T*>    KeyManager;
        xMemoryPool<T>         Pool;
    };

}
