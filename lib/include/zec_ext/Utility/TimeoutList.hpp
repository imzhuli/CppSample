#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec/Util/Chrono.hpp>

ZEC_NS
{

    class xTimeoutNode;
    template<typename T>
    class xTimeoutList;

    class xTimeoutNode
    : public xListNode
    {
    public:
        ZEC_INLINE uint64_t   GetTimestampMS() const { return _TimestampMS; }
    private:
        uint64_t   _TimestampMS = 0;
        xVariable  _UserContext = {};
        template<typename T>
        friend class xTimeoutList;
    };

    template<typename tNodeType = xTimeoutNode>
    class xTimeoutList final
    {
        static_assert(std::is_base_of_v<xTimeoutNode, tNodeType>);
    public:
        template<typename tCallback>
        ZEC_INLINE void PopTimeoutNodes(uint64_t TimeoutMS, tCallback && Callback) {
            uint64_t Now = GetMilliTimestamp();
            for(auto & Node : _TimeoutList) {
                if ((Now - Node._TimestampMS) <= TimeoutMS) {
                    break;
                }
                Node.Detach();
                std::forward<tCallback>(Callback)(Node, Node._UserContext);
            }
        }

        template<typename tCallback>
        ZEC_INLINE void Finish(tCallback && Callback) {
            for(auto & Node : _TimeoutList) {
                Node.Detach();
                std::forward<tCallback>(Callback)(Node, Node._UserContext);
            }
        }

        ZEC_INLINE void PushBack(xTimeoutNode & Node, xVariable UserContext = {})
        {
            Node._TimestampMS = GetMilliTimestamp();
            Node._UserContext = UserContext;
            _TimeoutList.GrabTail(Node);
        };

        ZEC_INLINE void Remove(xTimeoutNode & Node) {
            Node.Detach();
        }

        ZEC_INLINE bool IsEmpty() const {
            return _TimeoutList.IsEmpty();
        }

    private:
        xList<tNodeType>   _TimeoutList;
    };

}
