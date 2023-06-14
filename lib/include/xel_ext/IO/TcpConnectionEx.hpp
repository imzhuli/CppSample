#pragma once

#include <xel/Common.hpp>
#include <xel/Util/IndexedResourcePool.hpp>
#include "./TcpConnection.hpp"
#include <string>

X_NS
{

    struct xTcpConnectionExNode : xListNode, xNonCopyable
    {
        uint64_t ConnectionId  = 0;
        uint64_t ConnectionTimestampMS   = 0;
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
            virtual void   OnConnected(xAutoClientTcpConnection * ConnectionPtr) {}
            virtual size_t OnData(xAutoClientTcpConnection * ConnectionPtr, void * DataPtr, size_t DataSize) { return DataSize; }
        };

        X_API_MEMBER bool Init(xIoContext * ContextPtr, const xNetAddress ServerAddress, iListener * ListenerPtr);
        X_API_MEMBER void Clean();

        X_API_MEMBER void        Check(uint64_t NowMS);
        X_API_MEMBER void        ResetConnection() { _Dying = true; }
        X_API_MEMBER size_t      PostData(const void * DataPtr, size_t DataSize);
        X_API_MEMBER bool        IsConnected() const { return _Connected; }
        X_API_MEMBER uint64_t    GetVersion() const { return _Version; }

    protected:
        X_API_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr);
        X_API_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize);
        X_API_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr);
        X_API_MEMBER void   OnError(xTcpConnection * TcpConnectionPtr);

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
        bool Init(size_t PoolSize) {
            return ResourceManager.Init(PoolSize);
        }
        void Clean() {
            ResourceManager.Clean();
        }

        X_INLINE T* GetConnectionById(xIndexId Key) {
            return ResourceManager.GetInstanceByKey(Key);
        }

        X_INLINE T* CreateConnection() {
            auto [ConnectionPtr, ConnectionId] = ResourceManager.Create();
            if (!ConnectionPtr) {
                return nullptr;
            }
            ConnectionPtr->ConnectionId = ConnectionId;
            ConnectionPtr->ConnectionTimestampMS = GetTimestampMS();
            return ConnectionPtr;
        }

        X_INLINE void DestroyConnection(T* ConnectionPtr) {
            ResourceManager.Destroy(ConnectionPtr, ConnectionPtr->ConnectionId);
        }

    private:
        xIndexedResourcePool<T> ResourceManager;
    };

}
