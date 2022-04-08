#include <zec_ext/C/Net/Link.h>
#ifdef ZEC_SYSTEM_WINDOWS
    #define XelNoWriteSignal       0
#else
    #define XelNoWriteSignal       MSG_NOSIGNAL
#endif

#ifndef SOCK_CLOEXEC
    #define SOCK_CLOEXEC 0
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


/* Allocator */
static XelWriteBuffer * XWB_DefaultAlloc(void * CtxPtr)
{
    return (XelWriteBuffer*)malloc(sizeof(XelWriteBuffer));
}

static void XWB_DefaultFree(void * CtxPtr, XelWriteBuffer * BufferPtr)
{
    free(BufferPtr);
}

static XelWriteBuffer_Allocator XWB_DefaultAllocator = {
    &XWB_DefaultAlloc,
    &XWB_DefaultFree,
    NULL
};

XelWriteBuffer_Allocator * const XWB_DefaultAllocatorPtr = &XWB_DefaultAllocator;

/* Link Header */
static inline uint32_t MakeHeaderLength(uint32_t PacketLength) {
    assert(PacketLength <= XelMaxLinkPacketSize);
    return PacketLength | XelLinkMagicValue;
}

static inline bool CheckPackageLength(uint32_t PacketLength) {
    return (PacketLength & XelLinkMagicMask) == XelLinkMagicValue
        && (PacketLength & XelLinkLengthMask) <= XelMaxLinkPacketSize;
}

size_t XLH_Read(XelLinkHeader * HeaderPtr, const void * SourcePtr)
{
    XelStreamReaderContext Ctx = XSR(SourcePtr);
    HeaderPtr->PacketLength = XSR_4L(&Ctx);
    if (!CheckPackageLength(HeaderPtr->PacketLength)) {
        return 0;
    }
    HeaderPtr->PacketLength &= XelLinkLengthMask;
    HeaderPtr->PackageSequenceId        = XSR_1L(&Ctx);
    HeaderPtr->PackageSequenceTotalMax  = XSR_1L(&Ctx);
    HeaderPtr->CommandId                = XSR_2L(&Ctx);
    HeaderPtr->RequestId                = XSR_8L(&Ctx);
    XSR_Raw(&Ctx, HeaderPtr->TraceId, 16);
    return HeaderPtr->PacketLength;
}

void XLH_Write(const XelLinkHeader * HeaderPtr, void * DestPtr)
{
    XelStreamWriterContext Ctx = XSW(DestPtr);
    XSW_4L(&Ctx, MakeHeaderLength(HeaderPtr->PacketLength));
    XSW_1L(&Ctx, HeaderPtr->PackageSequenceId);
    XSW_1L(&Ctx, HeaderPtr->PackageSequenceTotalMax);
    XSW_2L(&Ctx, HeaderPtr->CommandId);
    XSW_8L(&Ctx, HeaderPtr->RequestId);
    XSW_Raw(&Ctx, HeaderPtr->TraceId, 16);
}

/* Link */
 bool XL_Connect(XelLink * LinkPtr, xel_in4 Addr, uint16_t Port)
 {
    assert(LinkPtr->Status == XLS_Idle);
    assert(LinkPtr->SocketFd == XelInvalidSocket);

    LinkPtr->SocketFd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (LinkPtr->SocketFd == XelInvalidSocket) {
        return false;
    }
    struct sockaddr_in TargetAddr = {};
    TargetAddr.sin_family = AF_INET;
    TargetAddr.sin_addr.s_addr = Addr;
    TargetAddr.sin_port = htons(Port);
    if (0 == connect(LinkPtr->SocketFd, (struct sockaddr*)&TargetAddr, sizeof(TargetAddr))) {
        LinkPtr->Status = XLS_Connected;
        return true;
    }
    if (errno != EAGAIN) {
        XelCloseSocket(LinkPtr->SocketFd);
        LinkPtr->SocketFd = XelInvalidSocket;
        return false;
    }
    LinkPtr->Status = XLS_Connecting;
    return true;
 }

bool XL_ReadRawData(XelLink * LinkPtr, void * DestBufferPtr, size_t * DestBufferSize)
{
    assert(XL_IsWorking(LinkPtr));
    assert(DestBufferPtr && DestBufferSize && *DestBufferSize);

    int Rb = read(LinkPtr->SocketFd, DestBufferPtr, *DestBufferSize);
    if (Rb == 0) {
        XEL_LINK_CALLBACK(LinkPtr, OnSetClose);
        return false;
    }
    if (Rb < 0) {
        if (errno == EAGAIN) {
            *DestBufferSize = 0;
            return true;
        }
        XL_SetError(LinkPtr);
        return false;
    }
    *DestBufferSize = 0;
    return true;
}

bool XL_ReadPacketLoop(XelLink * LinkPtr, XelPacketCallback * CallbackPtr, void * CallbackCtxPtr)
{
    assert(XL_IsWorking(LinkPtr));
    assert(CallbackPtr);

    int Rb = read(LinkPtr->SocketFd, LinkPtr->ReadBuffer + LinkPtr->ReadBufferDataSize, XelMaxLinkPacketSize - LinkPtr->ReadBufferDataSize);
    if (Rb == 0) { XEL_LINK_CALLBACK(LinkPtr, OnSetClose); return false; }
    if (Rb < 0) { return errno == EAGAIN; }
    LinkPtr->ReadBufferDataSize += Rb;

    xel_byte * StartPtr = LinkPtr->ReadBuffer;
    size_t RemainSize   = LinkPtr->ReadBufferDataSize;
    while(true) {
        if (RemainSize < XelLinkHeaderSize) {
            break;
        }
        XelLinkHeader Header = {};
        if (!XLH_Read(&Header, StartPtr)) {
            XL_SetError(LinkPtr);
            return false;
        }
        if (RemainSize < Header.PacketLength) {
            break;
        }
        if (!(*CallbackPtr)(CallbackCtxPtr, &Header, StartPtr + XelLinkHeaderSize, Header.PacketLength - XelLinkHeaderSize)) {
            XL_SetError(LinkPtr);
            return false;
        }
        StartPtr    += Header.PacketLength;
        RemainSize  -= Header.PacketLength;
    }
    if (RemainSize && StartPtr != LinkPtr->ReadBuffer) {
        memmove(LinkPtr->ReadBuffer, StartPtr, RemainSize);
        LinkPtr->ReadBufferDataSize = RemainSize;
    }
    return true;
}

bool XL_WriteRawData(XelLink * LinkPtr, const void * _DataPtr, size_t Length)
{
    // XS_PrintHexShow(stdout, _DataPtr, Length, true);
    if (!XL_IsWorking(LinkPtr)) {
        return false;
    }

    const xel_byte * DataPtr = _DataPtr;
    XelWriteBufferChain * ChainPtr = &LinkPtr->BufferChain;
    if (LinkPtr->Status == XLS_Connecting || XWBC_Peek(ChainPtr)) {
        return XL_AppendData(LinkPtr, DataPtr, Length);
    }
    int WB = send(LinkPtr->SocketFd, DataPtr, Length, MSG_NOSIGNAL);
    if (WB < 0) {
        if (errno != EAGAIN) {
            XL_SetError(LinkPtr);
            return false;
        }
        WB = 0;
    }
    DataPtr += WB;
    Length -= WB;
    return XL_AppendData(LinkPtr, DataPtr, Length);;
}

bool XL_ReadEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    // Do Nothing!
    return false;
}

void XL_ErrorEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    XL_SetError(LinkPtr);
}

bool XL_WriteEventCallback(void * CtxPtr, XelLink* LinkPtr)
{
    if (LinkPtr->Status == XLS_Connecting) {
        LinkPtr->Status = XLS_Connected;
    }
    return XL_FlushData(LinkPtr);
}

void XL_OnSetClose(void * CtxPtr, XelLink* LinkPtr)
{
    LinkPtr->Status = XEL_Closed;
}

void XL_SetError(XelLink* LinkPtr)
{
    LinkPtr->Flags |= XLF_ERROR;
    XEL_LINK_CALLBACK(LinkPtr, OnSetClose);
}
