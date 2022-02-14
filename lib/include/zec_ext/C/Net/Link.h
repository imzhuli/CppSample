#pragma once
#include <zec/C/Common.h>
#include <zec/C/Byte.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

#ifdef ZEC_SYSTEM_WINDOWS
#include <winsock2.h>
    typedef SOCKET XelSocket;
    #define XelCloseSocket(sockfd)     closesocket((sockfd))
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
    typedef int XelSocket;
    #define XelCloseSocket(sockfd)     close((sockfd))
#endif

#define XelLinkHeaderSize          ((size_t)(32))
#define XelLinkMagicMask           ((size_t)(0xFF000000))
#define XelLinkMagicValue          ((size_t)(0xCD000000))
#define XelLinkLengthMask          ((size_t)(0x00FFFFFF))
#define XelMaxLinkPacketSize       ((size_t)(4096 & XelLinkLengthMask))
#define XelMaxLinkPayloadSize      ((size_t)(XelMaxLinkPacketSize - XelLinkHeaderSize))

#define XelInvalidSocket           ((XelSocket) -1)

typedef uint32_t xel_in4;

/* Ip utils */
typedef union XelIpv4Addr {
    xel_in4 Addr;
    uint8_t Segs[4];
} XelIpv4Addr;

typedef struct XelIpv4Str {
    char Data[16];
} XelIpv4Str;

static inline XelIpv4Str Ip4ToStr(const xel_in4 SockAddrIn)
{
    XelIpv4Str Ret;
    XelIpv4Addr Punning;
    Punning.Addr = SockAddrIn;
    snprintf(Ret.Data, sizeof(Ret.Data), "%d.%d.%d.%d",
        (int)Punning.Segs[0],
        (int)Punning.Segs[1],
        (int)Punning.Segs[2],
        (int)Punning.Segs[3]);
    Ret.Data[15] = '\0';
    return Ret;
}

typedef struct XelLinkHeader {
    uint32_t     PacketLength; // header size included, lower 24 bits as length, higher 8 bits as a magic check
    uint8_t      PackageSequenceId; // the index of the packet in a full package, (this is no typo)
    uint8_t      PackageSequenceTotalMax;
    uint16_t     CommandId;
    uint64_t     RequestId;
    xel_byte     TraceId[16]; // allow uuid
} XelLinkHeader;

size_t XLH_Read(XelLinkHeader * HeaderPtr, const void * SourcePtr);
void   XLH_Write(const XelLinkHeader * HeaderPtr, void * DestPtr);

/* WriteBuffer(Chain) */
typedef struct XelWriteBuffer {
    xel_byte                   Buffer[XelMaxLinkPacketSize];
    size_t                     BufferDataSize;
    struct XelWriteBuffer *    NextPtr;
} XelWriteBuffer;

static inline void XWB_Init(XelWriteBuffer * BufferPtr) {
    BufferPtr->BufferDataSize = 0;
    BufferPtr->NextPtr = 0;
}

static inline void XWB_Clean(XelWriteBuffer * BufferPtr) {
    assert(!BufferPtr->NextPtr);
    BufferPtr->BufferDataSize = 0;
}

typedef struct {
    XelWriteBuffer * (*Alloc)(void *);
    void (*Free)(void *, XelWriteBuffer *);
    void * CtxPtr;
} XelWriteBuffer_Allocator;
extern XelWriteBuffer_Allocator * const XWB_DefaultAllocatorPtr;

typedef struct XelWriteBufferChain {
    XelWriteBuffer * FirstPtr;
    XelWriteBuffer * LastPtr;
    XelWriteBuffer_Allocator *  AllocatorPtr;
} XelWriteBufferChain;

/* link */
typedef enum {
    XLS_Idle = 0,
    XLS_Connecting,
    XLS_Connected,
    XEL_Closed,
} XelLinkStatus;

struct XelLinkCallbacks;

#define XLF_NONE      ((uint32_t)0x00000000)
#define XLF_ERROR     ((uint32_t)0x00000001)

typedef struct {
    XelLinkStatus               Status;
    uint32_t                    Flags;
    int                         SocketFd;
    xel_byte                    ReadBuffer[XelMaxLinkPacketSize];
    size_t                      ReadBufferDataSize;
    XelWriteBufferChain         BufferChain;
    struct XelLinkCallbacks *   CallbacksPtr;
} XelLink;

typedef struct XelLinkCallbacks {
    // system event callbacks
    bool (*OnReadEvent)(void * CtxPtr, XelLink* LinkPtr);
    bool (*OnWriteEvent)(void * CtxPtr, XelLink* LinkPtr);
    void (*OnErrorEvent)(void * CtxPtr, XelLink* LinkPtr);
    // callbacks during event procedures:
    void (*OnSetClose)(void * CtxPtr, XelLink* LinkPtr);
    void * CtxPtr;
} XelLinkCallbacks;
#define XEL_LINK_CALLBACK(LinkPtr, CallbackName) (*((LinkPtr)->CallbacksPtr->CallbackName))((LinkPtr)->CallbacksPtr->CtxPtr, (LinkPtr))

typedef bool XelPacketCallback(void * CtxPtr, const XelLinkHeader * HeaderPtr, const void * PayloadPtr, size_t PayloadSize);
extern bool XL_Connect(XelLink * LinkPtr, xel_in4 Addr, uint16_t Port);
extern bool XL_ReadRawData(XelLink * LinkPtr, void * DestBufferPtr, size_t * DestBufferSize);
extern bool XL_ReadPacketLoop(XelLink * LinkPtr, XelPacketCallback * CallbackPtr, void * CallbackCtxPtr);
extern bool XL_WriteRawData(XelLink * LinkPtr, const void * DataPtr, size_t Length);
extern void XL_SetError(XelLink* LinkPtr);
inline bool XL_IsError(XelLink* LinkPtr) { return LinkPtr->Flags & XLF_ERROR; }

extern bool XL_ReadEventCallback(void * CtxPtr, XelLink* LinkPtr);
extern bool XL_WriteEventCallback(void * CtxPtr, XelLink* LinkPtr);
extern void XL_ErrorEventCallback(void * CtxPtr, XelLink* LinkPtr);
extern void XL_OnSetClose(void * CtxPtr, XelLink* LinkPtr);

static inline XelWriteBuffer * XWBC_Alloc(XelWriteBufferChain * ChainPtr)
{
    return ChainPtr->AllocatorPtr->Alloc(ChainPtr->AllocatorPtr->CtxPtr);
}

static inline void XWBC_Free(XelWriteBufferChain * ChainPtr, XelWriteBuffer * BufferPtr)
{
    ChainPtr->AllocatorPtr->Free(ChainPtr->AllocatorPtr->CtxPtr, BufferPtr);
}

static inline XelWriteBuffer * XWBC_Peek(XelWriteBufferChain * ChainPtr)
{
    return ChainPtr->FirstPtr;
}

static inline void XWBC_FreeFront(XelWriteBufferChain * ChainPtr)
{
    XelWriteBuffer * BufferPtr = ChainPtr->FirstPtr;
    assert(BufferPtr);

    if (BufferPtr == ChainPtr->LastPtr) {
        ChainPtr->FirstPtr = ChainPtr->LastPtr = NULL;
    } else {
        ChainPtr->FirstPtr = BufferPtr->NextPtr;
    }
    BufferPtr->NextPtr = NULL;
    XWBC_Free(ChainPtr, BufferPtr);
}

static inline XelWriteBufferChain XWBC_Init(XelWriteBuffer_Allocator * AllocatorPtr)
{
    if (!AllocatorPtr) {
        AllocatorPtr = XWB_DefaultAllocatorPtr;
    }
    XelWriteBufferChain Ret = {
        NULL, NULL, AllocatorPtr
    };
    return Ret;
}

static inline void XWBC_Clean(XelWriteBufferChain * ChainPtr)
{
    while(XWBC_Peek(ChainPtr)) {
        XWBC_FreeFront(ChainPtr);
    }
}

static inline void XWBC_Append(XelWriteBufferChain * ChainPtr, XelWriteBuffer * BufferPtr)
{
    assert(BufferPtr);
    assert(!BufferPtr->NextPtr);
    if (!ChainPtr->FirstPtr) {
        ChainPtr->LastPtr = ChainPtr->FirstPtr = BufferPtr;
    } else {
        ChainPtr->LastPtr = BufferPtr;
    }
}

static inline bool XL_AppendData(XelLink * LinkPtr, const void * DataPtr, size_t DataSize)
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

ZEC_API bool XL_FlushData(XelLink * LinkPtr);


static inline bool XL_Init(XelLink * LinkPtr) {
    LinkPtr->Status = XLS_Idle;
    LinkPtr->Flags = 0;
    LinkPtr->SocketFd = XelInvalidSocket;
    LinkPtr->ReadBufferDataSize = 0;
    LinkPtr->BufferChain = XWBC_Init(NULL);
    return true;
}

static inline void XL_Clean(XelLink * LinkPtr) {
    if (LinkPtr->SocketFd != XelInvalidSocket) {
        XelCloseSocket(LinkPtr->SocketFd);
        LinkPtr->SocketFd = XelInvalidSocket;
    }
    LinkPtr->ReadBufferDataSize = 0;
    XWBC_Clean(&LinkPtr->BufferChain);
    LinkPtr->Status = XLS_Idle;
}

static inline bool XL_IsWorking(const XelLink* LinkPtr) {
    return LinkPtr->Status == XLS_Connecting || LinkPtr->Status == XLS_Connected;
}
