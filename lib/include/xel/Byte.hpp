#pragma once
#include "./Common.hpp"
#include <cstring>
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
	#define XelByteSwap16 OSSwapInt16
	#define XelByteSwap32 OSSwapInt32
	#define XelByteSwap64 OSSwapInt64
#elif defined(__ANDROID_API__)
    #define XelByteSwap16 __swap16
    #define XelByteSwap32 __swap32
    #define XelByteSwap64 __swap64
#elif defined(_POSIX_SOURCE) /* posix */
	#define XelByteSwap16 __bswap_16
	#define XelByteSwap32 __bswap_32
	#define XelByteSwap64 __bswap_64
#elif defined(_MSC_VER)
	#define XelByteSwap16 _byteswap_ushort
	#define XelByteSwap32 _byteswap_ulong
	#define XelByteSwap64 _byteswap_uint64
#endif

X_NS
{

	/****************************************
	* test if local endian is little endian
	*/
	#if BYTE_ORDER == LITTLE_ENDIAN
		#define X_IS_CONSIST_LITTLE_ENDIAN    true
		#define X_IS_CONSIST_BIG_ENDIAN       false

		X_STATIC_INLINE uint8_t  XelLE8 (const uint8_t s)  { return s; }
		X_STATIC_INLINE uint16_t XelLE16(const uint16_t s) { return s; }
		X_STATIC_INLINE uint32_t XelLE32(const uint32_t s) { return s; }
		X_STATIC_INLINE uint64_t XelLE64(const uint64_t s) { return s; }

		X_STATIC_INLINE uint8_t  XelBE8 (const uint8_t s)  { return s; }
		X_STATIC_INLINE uint16_t XelBE16(const uint16_t s) { return XelByteSwap16(s); }
		X_STATIC_INLINE uint32_t XelBE32(const uint32_t s) { return XelByteSwap32(s); }
		X_STATIC_INLINE uint64_t XelBE64(const uint64_t s) { return XelByteSwap64(s); }

	#elif BYTE_ORDER == BIG_ENDIAN
		#define X_IS_CONSIST_LITTLE_ENDIAN    false
		#define X_IS_CONSIST_BIG_ENDIAN       true

		X_STATIC_INLINE uint8_t  XelLE8 (const uint8_t s)  { return s; }
		X_STATIC_INLINE uint16_t XelLE16(const uint16_t s) { return XelByteSwap16(s); }
		X_STATIC_INLINE uint32_t XelLE32(const uint32_t s) { return XelByteSwap32(s); }
		X_STATIC_INLINE uint64_t XelLE64(const uint64_t s) { return XelByteSwap64(s); }

		X_STATIC_INLINE uint8_t  XelBE8 (const uint8_t s)  { return s; }
		X_STATIC_INLINE uint16_t XelBE16(const uint16_t s) { return s; }
		X_STATIC_INLINE uint32_t XelBE32(const uint32_t s) { return s; }
		X_STATIC_INLINE uint64_t XelBE64(const uint64_t s) { return s; }
	#else
		#error("Mixed endian is not supported by Xel");
	#endif

	namespace __detail__::__raw__{
		union UF {
			float f;
			uint32_t u;
		};
		union UD {
			double d;
			uint64_t u;
		};

		class iter
		{
		public:
			X_STATIC_INLINE void write16(ubyte * &p, const uint16_t & input) {
				xVariable V = { .U16 = input };
				*p++ = V.B[0];
				*p++ = V.B[1];
			}
			X_STATIC_INLINE void write32(ubyte * &p, const uint32_t & input) {
				xVariable V = { .U32 = input };
				*p++ = V.B[0];
				*p++ = V.B[1];
				*p++ = V.B[2];
				*p++ = V.B[3];
			}
			X_STATIC_INLINE void write64(ubyte * &p, const uint64_t & input) {
				xVariable V = { .U64 = input };
				*p++ = V.B[0];
				*p++ = V.B[1];
				*p++ = V.B[2];
				*p++ = V.B[3];
				*p++ = V.B[4];
				*p++ = V.B[5];
				*p++ = V.B[6];
				*p++ = V.B[7];
			}

			X_STATIC_INLINE uint16_t read16(const ubyte * &p) {
				xVariable V;
				V.B[0] = *p++;
				V.B[1] = *p++;
				return V.U16;
			}
			X_STATIC_INLINE uint32_t read32(const ubyte * &p) {
				xVariable V;
				V.B[0] = *p++;
				V.B[1] = *p++;
				V.B[2] = *p++;
				V.B[3] = *p++;
				return V.U32;
			}
			X_STATIC_INLINE uint64_t read64(const ubyte * &p) {
				xVariable V;
				V.B[0] = *p++;
				V.B[1] = *p++;
				V.B[2] = *p++;
				V.B[3] = *p++;
				V.B[4] = *p++;
				V.B[5] = *p++;
				V.B[6] = *p++;
				V.B[7] = *p++;
				return V.U64;
			}
		};
	}

	struct xStreamWriter final
	{
		using iter = __detail__::__raw__::iter;
	public:
		xStreamWriter() = default;
		X_INLINE xStreamWriter(void * p) { Reset(p);	}

		X_INLINE void W(ubyte c)                           { *(_curr++) = c; }
		X_INLINE void W(const void * s, ptrdiff_t len)     { ::memcpy(_curr, s, len); _curr += len; }
		X_INLINE void W0(ptrdiff_t len)                    { ::memset(_curr, 0, len); _curr += len; }

		X_INLINE void W1(uint8_t u)                        { *(_curr++) = u; }
		X_INLINE void W2(uint16_t u)                       { iter::write16(_curr, XelBE16(u)); }
		X_INLINE void W4(uint32_t u)                       { iter::write32(_curr, XelBE32(u)); }
		X_INLINE void W8(uint64_t u)                       { iter::write64(_curr, XelBE64(u)); }
		X_INLINE void Wf(float f)                          { __detail__::__raw__::UF uf{.f = f}; W4(uf.u); }
		X_INLINE void Wd(double d)                         { __detail__::__raw__::UD ud{.d = d}; W8(ud.u); }

		X_INLINE void W1L(uint8_t u)                       { *(_curr++) = u; }
		X_INLINE void W2L(uint16_t u)                      { iter::write16(_curr, XelLE16(u)); }
		X_INLINE void W4L(uint32_t u)                      { iter::write32(_curr, XelLE32(u)); }
		X_INLINE void W8L(uint64_t u)                      { iter::write64(_curr, XelLE64(u)); }
		X_INLINE void WFL(float f)                         { __detail__::__raw__::UF uf{.f = f}; W4L(uf.u); }
		X_INLINE void WDL(double d)                        { __detail__::__raw__::UD ud{.d = d}; W8L(ud.u); }

		X_INLINE void *          operator ()() const          { return _curr; }
		X_INLINE operator        void * () const              { return _curr; }
		X_INLINE void *          Origin() const               { return _start; }
		X_INLINE ptrdiff_t       Offset() const               { return _curr - _start; }
		X_INLINE void            Offset(ptrdiff_t offset)     { _curr = _start + offset; }
		X_INLINE void *          Skip(ptrdiff_t len)          { ubyte * from = _curr; _curr += len; return from; }

		X_INLINE void Reset()                              { _curr = _start; }
		X_INLINE void Reset(void * s)                      { _curr = _start = static_cast<ubyte *>(s); }

	private:
		ubyte * _curr = nullptr;
		ubyte * _start = nullptr;
	};

	struct xStreamReader final
	{
		using iter = __detail__::__raw__::iter;
	public:
		xStreamReader() = default;
		X_INLINE xStreamReader(const void * p) { Reset(p); }

		X_INLINE ubyte    R()                              { return *(_curr++); }
		X_INLINE void     R(void * d, ptrdiff_t len)       { ::memcpy(d, _curr, len); _curr += len; }

		X_INLINE uint8_t  R1()                             { return *(_curr++); }
		X_INLINE uint16_t R2()                             { return XelBE16(iter::read16(_curr)); }
		X_INLINE uint32_t R4()                             { return XelBE32(iter::read32(_curr)); }
		X_INLINE uint64_t R8()                             { return XelBE64(iter::read64(_curr)); }
		X_INLINE float    RF()                             { __detail__::__raw__::UF uf{.u=R4()}; return uf.f; }
		X_INLINE double   RD()                             { __detail__::__raw__::UD ud{.u=R8()}; return ud.d; }

		X_INLINE uint8_t  R1L()                            { return *(_curr++); }
		X_INLINE uint16_t R2L()                            { return XelLE16(iter::read16(_curr)); }
		X_INLINE uint32_t R4L()                            { return XelLE32(iter::read32(_curr)); }
		X_INLINE uint64_t R8L()                            { return XelLE64(iter::read64(_curr)); }
		X_INLINE float    RFL()                            { __detail__::__raw__::UF uf{.u=R4L()}; return uf.f; }
		X_INLINE double   RDL()                            { __detail__::__raw__::UD ud{.u=R8L()}; return ud.d; }

		X_INLINE const void *      operator ()() const        { return _curr; }
		X_INLINE operator          const void * () const      { return _curr; }
		X_INLINE const void *     Origin() const              { return _start; }
		X_INLINE ptrdiff_t         Offset() const             { return _curr - _start; }
		X_INLINE void              Offset(ptrdiff_t offset)   { _curr = _start + offset; }
		X_INLINE const void *     Skip(ptrdiff_t len)         { const ubyte * from = _curr; _curr += len; return from; }

		X_INLINE void Reset()                              { _curr = _start; }
		X_INLINE void Reset(const void * s)                { _curr = _start = static_cast<const ubyte *>(s); }

	private:
		const ubyte * _curr = nullptr;
		const ubyte * _start = nullptr;
	};

}