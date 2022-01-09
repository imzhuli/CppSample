#pragma once
#include <string.h>
#include <stdint.h>

#if defined(__APPLE__)
	#include <architecture/byte_order.h>
	#include <libkern/OSByteOrder.h>
#elif defined(__ANDROID_API__)
    #include <endian.h>
    #include <byteswap.h>
#elif defined(_POSIX_SOURCE)
	#include <endian.h>
	#include <byteswap.h>
#elif defined(_MSC_VER)
	#include <stdlib.h>
#else
	#error "no supported byte order operations yet"
#endif

#if defined(__APPLE__)
	#define xel_byteSwap16 OSSwapInt16
	#define xel_byteSwap32 OSSwapInt32
	#define xel_byteSwap64 OSSwapInt64
#elif defined(__ANDROID_API__)
    #define xel_byteSwap16 __swap16
    #define xel_byteSwap32 __swap32
    #define xel_byteSwap64 __swap64
#elif defined(_POSIX_SOURCE)
	#define xel_byteSwap16 __bswap_16
	#define xel_byteSwap32 __bswap_32
	#define xel_byteSwap64 __bswap_64
#elif defined(_MSC_VER)
	#define xel_byteSwap16 _byteswap_ushort
	#define xel_byteSwap32 _byteswap_ulong
	#define xel_byteSwap64 _byteswap_uint64
#endif

/****************************************
* test if local endian is little endian
*/
#if BYTE_ORDER == LITTLE_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    true
	#define Xel_IS_CONSIST_BIG_ENDIAN       false
	static inline uint8_t  XelLE8 (const uint8_t s)  { return s; }
	static inline uint16_t XelLE16(const uint16_t s) { return s; }
	static inline uint32_t XelLE32(const uint32_t s) { return s; }
	static inline uint64_t XelLE64(const uint64_t s) { return s; }
	static inline uint8_t  XelBE8 (const uint8_t s)  { return s; }
	static inline uint16_t XelBE16(const uint16_t s) { return xel_byteSwap16(s); }
	static inline uint32_t XelBE32(const uint32_t s) { return xel_byteSwap32(s); }
	static inline uint64_t XelBE64(const uint64_t s) { return xel_byteSwap64(s); }
#elif BYTE_ORDER == BIG_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    false
	#define Xel_IS_CONSIST_BIG_ENDIAN       true
	static inline uint8_t  XelLE8 (const uint8_t s)  { return s; }
	static inline uint16_t XelLE16(const uint16_t s) { return xel_byteSwap16(s); }
	static inline uint32_t XelLE32(const uint32_t s) { return xel_byteSwap32(s); }
	static inline uint64_t XelLE64(const uint64_t s) { return xel_byteSwap64(s); }
	static inline uint8_t  XelBE8 (const uint8_t s)  { return s; }
	static inline uint16_t XelBE16(const uint16_t s) { return s; }
	static inline uint32_t XelBE32(const uint32_t s) { return s; }
	static inline uint64_t XelBE64(const uint64_t s) { return s; }
#else
	#error("Mixed endian is not supported by Xel");
#endif

typedef unsigned char xel_byte;

typedef union {
	uint8_t  _1;
	uint16_t _2;
    uint32_t _4;
	uint64_t _8;
	xel_byte _[8];
} XelBytePunning;

typedef struct {
	const xel_byte * Start;
	const xel_byte * Current;
} XelStreamReaderContext;

typedef struct {
	xel_byte * Start;
	xel_byte * Current;
} XelStreamWriterContext;

/* StreamReader */
static inline XelStreamReaderContext SR(const void * SourcePtr) {
	XelStreamReaderContext Ctx;
	Ctx.Current = Ctx.Start = (const xel_byte *)SourcePtr;
	return Ctx;
}
static inline void RRewind(XelStreamReaderContext * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
static inline xel_byte RB(XelStreamReaderContext * CtxPtr) {
	return (*(CtxPtr->Current++));
}
static inline uint8_t R1(XelStreamReaderContext * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
static inline uint16_t R2(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return XelBE16(Punning._2);
}
static inline uint32_t R4(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return XelBE32(Punning._4);
}
static inline uint64_t R8(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return XelBE64(Punning._8);
}
static inline uint8_t R1L(XelStreamReaderContext * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
static inline uint16_t R2L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return XelLE16(Punning._2);
}
static inline uint32_t R4L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return XelLE32(Punning._4);
}
static inline uint64_t R8L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return XelLE64(Punning._8);
}
static inline void R(XelStreamReaderContext * CtxPtr, void * DestPtr, size_t Length) {
	memcpy(DestPtr, CtxPtr->Current, Length);
	CtxPtr->Current += Length;
}
static inline void RSkip(XelStreamReaderContext * CtxPtr, size_t Length) {
	CtxPtr->Current += Length;
}
static inline size_t RPos(XelStreamReaderContext * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}

/* StreamWriter */
static inline XelStreamWriterContext SW(void * SourcePtr) {
	XelStreamWriterContext Ctx;
	Ctx.Current = Ctx.Start = (xel_byte *)SourcePtr;
	return Ctx;
}
static inline void WRewind(XelStreamWriterContext * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
static inline void WB(XelStreamWriterContext * CtxPtr, xel_byte Value) {
	(*(CtxPtr->Current++)) = Value;
}
static inline void W1(XelStreamWriterContext * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (xel_byte)Value;
}
static inline void W2(XelStreamWriterContext * CtxPtr, uint16_t Value) {
	XelBytePunning Punning;
	Punning._2 = XelBE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
static inline void W4(XelStreamWriterContext * CtxPtr, uint32_t Value) {
	XelBytePunning Punning;
	Punning._4 = XelBE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
static inline void W8(XelStreamWriterContext * CtxPtr, uint64_t Value) {
	XelBytePunning Punning;
	Punning._8 = XelBE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
static inline void W1L(XelStreamWriterContext * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (xel_byte)Value;
}
static inline void W2L(XelStreamWriterContext * CtxPtr, uint16_t Value) {
	XelBytePunning Punning;
	Punning._2 = XelLE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
static inline void W4L(XelStreamWriterContext * CtxPtr, uint32_t Value) {
	XelBytePunning Punning;
	Punning._4 = XelLE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
static inline void W8L(XelStreamWriterContext * CtxPtr, uint64_t Value) {
	XelBytePunning Punning;
	Punning._8 = XelLE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
static inline void W(XelStreamWriterContext * CtxPtr, const void * SourcePtr, size_t Length) {
	memcpy(CtxPtr->Current, SourcePtr, Length);
	CtxPtr->Current += Length;
}
static inline void WSkip(XelStreamWriterContext * CtxPtr, size_t Length) {
	CtxPtr->Current += Length;
}
static inline size_t WPos(XelStreamWriterContext * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}
