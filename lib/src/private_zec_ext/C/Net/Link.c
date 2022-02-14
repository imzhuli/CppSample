#include <zec_ext/C/Net/Link.h>
#ifdef ZEC_SYSTEM_WINDOWS
    #define XelNoWriteSignal       0
#else
    #define XelNoWriteSignal       MSG_NOSIGNAL
#endif

bool XL_Init(XelLink * LinkPtr)
{
    LinkPtr->Status = XLS_Idle;
    LinkPtr->Flags = 0;
    LinkPtr->SocketFd = XelInvalidSocket;
    LinkPtr->ReadBufferDataSize = 0;
    LinkPtr->BufferChain = XWBC_Init(NULL);
    return true;
}

void XL_Clean(XelLink * LinkPtr)
{
    if (LinkPtr->SocketFd != XelInvalidSocket) {
        XelCloseSocket(LinkPtr->SocketFd);
        LinkPtr->SocketFd = XelInvalidSocket;
    }
    LinkPtr->ReadBufferDataSize = 0;
    XWBC_Clean(&LinkPtr->BufferChain);
    LinkPtr->Status = XLS_Idle;
}

bool XL_AppendData(XelLink * LinkPtr, const void * DataPtr, size_t DataSize)
{
    XelWriteBufferChain * ChainPtr = &LinkPtr->BufferChain;
    const xel_byte * Cursor = DataPtr;
    size_t RemainSize = DataSize;
    while(RemainSize) {
        XelWriteBuffer * BufferPtr = XWBC_Alloc(ChainPtr);
        if (!BufferPtr) {
            XL_SetError(LinkPtr);
            return false;
        }
        size_t CopySize = RemainSize < XelMaxLinkPacketSize ? RemainSize : XelMaxLinkPacketSize;
        memcpy(BufferPtr->Buffer, Cursor, CopySize);
        BufferPtr->BufferDataSize = CopySize;
        XWBC_Append(ChainPtr, BufferPtr);
        Cursor += CopySize;
        RemainSize -= CopySize;
    }
    return true;
}

bool XL_FlushData(XelLink * LinkPtr)
{
    assert(LinkPtr->Status == XLS_Connected);
    XelWriteBufferChain * ChainPtr = &LinkPtr->BufferChain;
    while(true) {
        XelWriteBuffer * BufferPtr = XWBC_Peek(ChainPtr);
        if (!BufferPtr) {
            return true;
        }
        int WB = send(LinkPtr->SocketFd, BufferPtr->Buffer, BufferPtr->BufferDataSize, XelNoWriteSignal);
        if (WB == BufferPtr->BufferDataSize) {
            XWBC_FreeFront(ChainPtr);
            continue;
        }
        if (WB < 0) {
            if (errno != EAGAIN) {
                XL_SetError(LinkPtr);
                return false;
            }
            break;
        }
        if ((size_t)WB < BufferPtr->BufferDataSize) {
            BufferPtr->BufferDataSize -= WB;
            memmove(BufferPtr->Buffer, BufferPtr->Buffer + WB, BufferPtr->BufferDataSize);
            break;
        }
    }
    return true;
}
