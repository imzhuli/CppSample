#include <zec_ext/C/Net/Link.h>
#ifdef ZEC_SYSTEM_WINDOWS
    #define XelNoWriteSignal       0
#else
    #define XelNoWriteSignal       MSG_NOSIGNAL
#endif

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
