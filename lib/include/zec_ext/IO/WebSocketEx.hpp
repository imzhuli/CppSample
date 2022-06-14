#pragma once
#include "./WebSocket.hpp"

ZEC_NS
{
    struct xWebSocketSessionExNode: xListNode {
        uint64_t SessionTimestampMS = 0;
        uint64_t SessionId = 0;
    };
    using xWebSocketSessionList = xList<xWebSocketSessionExNode>;

    class xWebSocketSessionEx
    : public xWebSocketSession
    , public xWebSocketSessionExNode
    {
    public:
        ZEC_INLINE void Detach() { xWebSocketSessionExNode::Detach(); }
    };

}
