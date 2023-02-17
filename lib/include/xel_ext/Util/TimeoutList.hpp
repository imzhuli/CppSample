#pragma once
#include <xel/Common.hpp>
#include <xel/List.hpp>
#include <xel/Util/Chrono.hpp>

X_NS
{

    class xTimeoutNode;
    template<typename T>
    class xTimeoutList;

    class xTimeoutNode
    : public xListNode
    {
    public:
        X_INLINE uint64_t   GetTimestampMS() const { return _TimestampMS; }
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
        X_INLINE void PopTimeoutNodes(uint64_t TimeoutMS, tCallback && Callback) {
            uint64_t Now = GetTimestampMS();
            for(auto & Node : _TimeoutList) {
                if ((Now - Node._TimestampMS) <= TimeoutMS) {
                    break;
                }
                Node.Detach();
                std::forward<tCallback>(Callback)(Node, Node._UserContext);
            }
        }

        template<typename tCallback>
        X_INLINE void Finish(tCallback && Callback) {
            for(auto & Node : _TimeoutList) {
                Node.Detach();
                std::forward<tCallback>(Callback)(Node, Node._UserContext);
            }
        }

        X_INLINE void PushBack(xTimeoutNode & Node, xVariable UserContext = {})
        {
            Node._TimestampMS = GetTimestampMS();
            Node._UserContext = UserContext;
            _TimeoutList.GrabTail(Node);
        };

        X_INLINE void Remove(xTimeoutNode & Node) {
            Node.Detach();
        }

        X_INLINE bool IsEmpty() const {
            return _TimeoutList.IsEmpty();
        }

    private:
        xList<tNodeType>   _TimeoutList;
    };

}
