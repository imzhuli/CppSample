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
	#define X_ByteSwap16 OSSwapInt16
	#define X_ByteSwap32 OSSwapInt32
	#define X_ByteSwap64 OSSwapInt64
#elif defined(__ANDROID_API__)
    #define X_ByteSwap16 __swap16
    #define X_ByteSwap32 __swap32
    #define X_ByteSwap64 __swap64
#elif defined(_POSIX_SOURCE)
	#define X_ByteSwap16 __bswap_16
	#define X_ByteSwap32 __bswap_32
	#define X_ByteSwap64 __bswap_64
#elif defined(_MSC_VER)
	#define X_ByteSwap16 _byteswap_ushort
	#define X_ByteSwap32 _byteswap_ulong
	#define X_ByteSwap64 _byteswap_uint64
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************
* test if local endian is little endian
*/
#if BYTE_ORDER == LITTLE_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    true
	#define Xel_IS_CONSIST_BIG_ENDIAN       false
	static inline uint8_t  X_LE8 (const uint8_t s)  { return s; }
	static inline uint16_t X_LE16(const uint16_t s) { return s; }
	static inline uint32_t X_LE32(const uint32_t s) { return s; }
	static inline uint64_t X_LE64(const uint64_t s) { return s; }
	static inline uint8_t  X_BE8 (const uint8_t s)  { return s; }
	static inline uint16_t X_BE16(const uint16_t s) { return X_ByteSwap16(s); }
	static inline uint32_t X_BE32(const uint32_t s) { return X_ByteSwap32(s); }
	static inline uint64_t X_BE64(const uint64_t s) { return X_ByteSwap64(s); }
#elif BYTE_ORDER == BIG_ENDIAN
	#define Xel_IS_CONSIST_LITTLE_ENDIAN    false
	#define Xel_IS_CONSIST_BIG_ENDIAN       true
	static inline uint8_t  X_LE8 (const uint8_t s)  { return s; }
	static inline uint16_t X_LE16(const uint16_t s) { return X_ByteSwap16(s); }
	static inline uint32_t X_LE32(const uint32_t s) { return X_ByteSwap32(s); }
	static inline uint64_t X_LE64(const uint64_t s) { return X_ByteSwap64(s); }
	static inline uint8_t  X_BE8 (const uint8_t s)  { return s; }
	static inline uint16_t X_BE16(const uint16_t s) { return s; }
	static inline uint32_t X_BE32(const uint32_t s) { return s; }
	static inline uint64_t X_BE64(const uint64_t s) { return s; }
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
	const xel_byte * Current;
	const xel_byte * Start;
} XelStreamReaderContext;

typedef struct {
	xel_byte * Current;
	xel_byte * Start;
} XelStreamWriterContext;

/* StreamReader */
static inline XelStreamReaderContext XSR(const void * SourcePtr) {
	XelStreamReaderContext Ctx;
	Ctx.Current = Ctx.Start = (const xel_byte *)SourcePtr;
	return Ctx;
}
static inline void XSR_Rewind(XelStreamReaderContext * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
static inline xel_byte XSR_B(XelStreamReaderContext * CtxPtr) {
	return (*(CtxPtr->Current++));
}
static inline uint8_t XSR_1(XelStreamReaderContext * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
static inline uint16_t XSR_2(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return X_BE16(Punning._2);
}
static inline uint32_t XSR_4(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return X_BE32(Punning._4);
}
static inline uint64_t XSR_8(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return X_BE64(Punning._8);
}
static inline uint8_t XSR_1L(XelStreamReaderContext * CtxPtr) {
	return (uint8_t)(*(CtxPtr->Current++));
}
static inline uint16_t XSR_2L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	return X_LE16(Punning._2);
}
static inline uint32_t XSR_4L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	return X_LE32(Punning._4);
}
static inline uint64_t XSR_8L(XelStreamReaderContext * CtxPtr) {
	XelBytePunning Punning;
	Punning._[0] = (*(CtxPtr->Current++));
	Punning._[1] = (*(CtxPtr->Current++));
	Punning._[2] = (*(CtxPtr->Current++));
	Punning._[3] = (*(CtxPtr->Current++));
	Punning._[4] = (*(CtxPtr->Current++));
	Punning._[5] = (*(CtxPtr->Current++));
	Punning._[6] = (*(CtxPtr->Current++));
	Punning._[7] = (*(CtxPtr->Current++));
	return X_LE64(Punning._8);
}
static inline void XSR_Raw(XelStreamReaderContext * CtxPtr, void * DestPtr, size_t Length) {
	memcpy(DestPtr, CtxPtr->Current, Length);
	CtxPtr->Current += Length;
}
static inline void XSR_Skip(XelStreamReaderContext * CtxPtr, size_t Length) {
	CtxPtr->Current += Length;
}
static inline size_t XSR_Pos(XelStreamReaderContext * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}

/* StreamWriter */
static inline XelStreamWriterContext XSW(void * SourcePtr) {
	XelStreamWriterContext Ctx;
	Ctx.Current = Ctx.Start = (xel_byte *)SourcePtr;
	return Ctx;
}
static inline void XSW_Rewind(XelStreamWriterContext * CtxPtr) {
	CtxPtr->Current = CtxPtr->Start;
}
static inline void XSW_B(XelStreamWriterContext * CtxPtr, xel_byte Value) {
	(*(CtxPtr->Current++)) = Value;
}
static inline void XSW_1(XelStreamWriterContext * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (xel_byte)Value;
}
static inline void XSW_2(XelStreamWriterContext * CtxPtr, uint16_t Value) {
	XelBytePunning Punning;
	Punning._2 = X_BE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
static inline void XSW_4(XelStreamWriterContext * CtxPtr, uint32_t Value) {
	XelBytePunning Punning;
	Punning._4 = X_BE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
static inline void XSW_8(XelStreamWriterContext * CtxPtr, uint64_t Value) {
	XelBytePunning Punning;
	Punning._8 = X_BE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
static inline void XSW_1L(XelStreamWriterContext * CtxPtr, uint8_t Value) {
	(*(CtxPtr->Current++)) = (xel_byte)Value;
}
static inline void XSW_2L(XelStreamWriterContext * CtxPtr, uint16_t Value) {
	XelBytePunning Punning;
	Punning._2 = X_LE16(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
}
static inline void XSW_4L(XelStreamWriterContext * CtxPtr, uint32_t Value) {
	XelBytePunning Punning;
	Punning._4 = X_LE32(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
}
static inline void XSW_8L(XelStreamWriterContext * CtxPtr, uint64_t Value) {
	XelBytePunning Punning;
	Punning._8 = X_LE64(Value);
	(*(CtxPtr->Current++)) = Punning._[0];
	(*(CtxPtr->Current++)) = Punning._[1];
	(*(CtxPtr->Current++)) = Punning._[2];
	(*(CtxPtr->Current++)) = Punning._[3];
	(*(CtxPtr->Current++)) = Punning._[4];
	(*(CtxPtr->Current++)) = Punning._[5];
	(*(CtxPtr->Current++)) = Punning._[6];
	(*(CtxPtr->Current++)) = Punning._[7];
}
static inline void XSW_Raw(XelStreamWriterContext * CtxPtr, const void * SourcePtr, size_t Length) {
	memcpy(CtxPtr->Current, SourcePtr, Length);
	CtxPtr->Current += Length;
}
static inline void XSW_Skip(XelStreamWriterContext * CtxPtr, size_t Length) {
	CtxPtr->Current += Length;
}
static inline size_t XSW_Pos(XelStreamWriterContext * CtxPtr) {
	return (CtxPtr->Current - CtxPtr->Start);
}

#ifdef __cplusplus
}
#endif
