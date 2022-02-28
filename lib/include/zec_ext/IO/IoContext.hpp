#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec/Util/Chrono.hpp>
#include <string>

ZEC_NS
{

    namespace __detail__ {
        class IOUtil;
    }

    class xIoNativeHandle
    {
    public:
        ZEC_INLINE xIoNativeHandle(void * NativeObjectPtr) : ObjectPtr(NativeObjectPtr) {}
        template<typename T>
        ZEC_INLINE T* GetPtrAs() const {
            return static_cast<T*>(ObjectPtr);
        };
        template<typename T>
        ZEC_INLINE T& GetRefAs() const {
            return *static_cast<T*>(ObjectPtr);
        };

    private:
        void * ObjectPtr;

        friend class xTcpConnection;
        friend class xTcpServer;
    };

    class xIoContext
    : xNonCopyable
    {
    public:
        struct xIoExpiringNode : xListNode {
        protected:
            virtual void OnFinalIoExpired();
        private:
            uint64_t TimestampMS = 0;
            friend xIoContext;
        };

    public:
        ZEC_API_MEMBER bool Init(uint64_t RemoveTimeMS = 3000);
        ZEC_API_MEMBER void Clean();
        ZEC_API_MEMBER void LoopOnce(int TimeoutMS);
        ZEC_INLINE void SetFinalExpiration(xIoExpiringNode & Node) { Node.TimestampMS = GetMicroTimestamp(); _ExpiringNodelist.GrabTail(Node); }

    private:
        xDummy<16>                  _Native;
        uint64_t                    _ExpireTimeout;
        xList<xIoExpiringNode>      _ExpiringNodelist;
        friend class __detail__::IOUtil;
    };

    struct xNetAddress {

        enum : uint8_t {
            eUnknown, eIpv4, eIpv6
        } Type = eUnknown;

        union {
            ubyte Ipv4[4];
            ubyte Ipv6[16];
        };

        ZEC_INLINE bool IsV4() const { return Type == eIpv4; }
        ZEC_INLINE bool IsV6() const { return Type == eIpv6; }
        ZEC_INLINE operator bool () const { return Type != eUnknown; }
        ZEC_API_MEMBER std::string ToString() const;

        ZEC_API_STATIC_MEMBER xNetAddress Make(const char * IpStr);
        ZEC_API_STATIC_MEMBER xNetAddress MakeV4(const char * IpStr);
        ZEC_API_STATIC_MEMBER xNetAddress MakeV6(const char * IpStr);
    };

}
