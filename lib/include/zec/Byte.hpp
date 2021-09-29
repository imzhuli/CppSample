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
	#define zecByteSwap16 OSSwapInt16
	#define zecByteSwap32 OSSwapInt32
	#define zecByteSwap64 OSSwapInt64
#elif defined(__ANDROID_API__)
    #define zecByteSwap16 __swap16
    #define zecByteSwap32 __swap32
    #define zecByteSwap64 __swap64
#elif defined(_POSIX_SOURCE) /* posix */
	#define zecByteSwap16 __bswap_16
	#define zecByteSwap32 __bswap_32
	#define zecByteSwap64 __bswap_64
#elif defined(_MSC_VER)
	#define zecByteSwap16 _byteswap_ushort
	#define zecByteSwap32 _byteswap_ulong
	#define zecByteSwap64 _byteswap_uint64
#endif

ZEC_NS
{

	/****************************************
	* test if local endian is little endian
	*/
	#if BYTE_ORDER == LITTLE_ENDIAN
		#define ZEC_IS_CONSIST_LITTLE_ENDIAN    true
		#define ZEC_IS_CONSIST_BIG_ENDIAN       false

		ZEC_STATIC_INLINE uint8_t  zecLE8 (const uint8_t s)  { return s; }
		ZEC_STATIC_INLINE uint16_t zecLE16(const uint16_t s) { return s; }
		ZEC_STATIC_INLINE uint32_t zecLE32(const uint32_t s) { return s; }
		ZEC_STATIC_INLINE uint64_t zecLE64(const uint64_t s) { return s; }

		ZEC_STATIC_INLINE uint8_t  zecBE8 (const uint8_t s)  { return s; }
		ZEC_STATIC_INLINE uint16_t zecBE16(const uint16_t s) { return zecByteSwap16(s); }
		ZEC_STATIC_INLINE uint32_t zecBE32(const uint32_t s) { return zecByteSwap32(s); }
		ZEC_STATIC_INLINE uint64_t zecBE64(const uint64_t s) { return zecByteSwap64(s); }

	#elif BYTE_ORDER == BIG_ENDIAN
		#define ZEC_IS_CONSIST_LITTLE_ENDIAN    false
		#define ZEC_IS_CONSIST_BIG_ENDIAN       true

		ZEC_STATIC_INLINE uint8_t  zecLE8 (const uint8_t s)  { return s; }
		ZEC_STATIC_INLINE uint16_t zecLE16(const uint16_t s) { return zecByteSwap16(s); }
		ZEC_STATIC_INLINE uint32_t zecLE32(const uint32_t s) { return zecByteSwap32(s); }
		ZEC_STATIC_INLINE uint64_t zecLE64(const uint64_t s) { return zecByteSwap64(s); }

		ZEC_STATIC_INLINE uint8_t  zecBE8 (const uint8_t s)  { return s; }
		ZEC_STATIC_INLINE uint16_t zecBE16(const uint16_t s) { return s; }
		ZEC_STATIC_INLINE uint32_t zecBE32(const uint32_t s) { return s; }
		ZEC_STATIC_INLINE uint64_t zecBE64(const uint64_t s) { return s; }
	#else
		#error("Mixed endian is not supported by zec");
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
		private:
			template<typename T>
			union U {
				T i;
				ubyte bytes[sizeof(T)];
				static_assert(std::is_integral_v<T>);
			};
		public:
			template<typename T>
			ZEC_STATIC_INLINE void write(ubyte * &p, const T & input) {
				U<T> u;
				u.i = input;
				memcpy(p, u.bytes, sizeof(T));
				p += sizeof(T);
			}
			template<typename T>
			ZEC_STATIC_INLINE T read(const ubyte * &p) {
				U<T> u;
				memcpy(u.bytes, p, sizeof(T));
				p += sizeof(T);
				return u.i;
			}
		};
	}

	struct xStreamWriter final
	{
		using iter = __detail__::__raw__::iter;
	public:
		xStreamWriter() = default;
		ZEC_INLINE xStreamWriter(void * p) { Reset(p);	}

		ZEC_INLINE void W(char c)                            { *(_curr++) = c; }
		ZEC_INLINE void W(const void * s, ptrdiff_t len)     { ::memcpy(_curr, s, len); _curr += len; }

		ZEC_INLINE void W1(uint8_t u)                        { *(_curr++) = u; }
		ZEC_INLINE void W2(uint16_t u)                       { iter::write(_curr, zecBE16(u)); }
		ZEC_INLINE void W4(uint32_t u)                       { iter::write(_curr, zecBE32(u)); }
		ZEC_INLINE void W8(uint64_t u)                       { iter::write(_curr, zecBE64(u)); }
		ZEC_INLINE void Wf(float f)                          { __detail__::__raw__::UF uf{.f = f}; W4(uf.u); }
		ZEC_INLINE void Wd(double d)                         { __detail__::__raw__::UD ud{.d = d}; W8(ud.u); }

		ZEC_INLINE void W1l(uint8_t u)                       { *(_curr++) = u; }
		ZEC_INLINE void W2l(uint16_t u)                      { iter::write(_curr, zecLE16(u)); }
		ZEC_INLINE void W4l(uint32_t u)                      { iter::write(_curr, zecLE32(u)); }
		ZEC_INLINE void W8l(uint64_t u)                      { iter::write(_curr, zecLE64(u)); }
		ZEC_INLINE void Wfl(float f)                         { __detail__::__raw__::UF uf{.f = f}; W4l(uf.u); }
		ZEC_INLINE void Wdl(double d)                        { __detail__::__raw__::UD ud{.d = d}; W8l(ud.u); }

		ZEC_INLINE void *          operator ()() const          { return _curr; }
		ZEC_INLINE operator        ubyte * () const             { return _curr; }
		ZEC_INLINE ubyte *         Origin() const               { return _start; }
		ZEC_INLINE ptrdiff_t       Offset() const               { return _curr - _start; }
		ZEC_INLINE ubyte *         Skip(ptrdiff_t len = 0)      { ubyte * from = _curr; _curr += len; return from; }

		ZEC_INLINE void Reset()                              { _curr = _start; }
		ZEC_INLINE void Reset(void * s)                      { _curr = _start = static_cast<ubyte *>(s); }

	private:
		ubyte * _curr = nullptr;
		ubyte * _start = nullptr;
	};

	struct xStreamReader final
	{
		using iter = __detail__::__raw__::iter;
	public:
		xStreamReader() = default;
		ZEC_INLINE xStreamReader(const void * p) { Reset(p); }

		ZEC_INLINE char     R()                              { return *(_curr++); }
		ZEC_INLINE void     R(void * d, ptrdiff_t zecLEn)       { ::memcpy(d, _curr, zecLEn); _curr += zecLEn; }

		ZEC_INLINE uint8_t  R1()                             { return *(_curr++); }
		ZEC_INLINE uint16_t R2()                             { return zecBE16(iter::read<uint16_t>(_curr)); }
		ZEC_INLINE uint32_t R4()                             { return zecBE32(iter::read<uint32_t>(_curr)); }
		ZEC_INLINE uint64_t R8()                             { return zecBE64(iter::read<uint64_t>(_curr)); }
		ZEC_INLINE float    RF()                             { __detail__::__raw__::UF uf{.u=R4()}; return uf.f; }
		ZEC_INLINE double   RD()                             { __detail__::__raw__::UD ud{.u=R8()}; return ud.d; }

		ZEC_INLINE uint8_t  R1l()                            { return *(_curr++); }
		ZEC_INLINE uint16_t R2l()                            { return zecLE16(iter::read<uint16_t>(_curr)); }
		ZEC_INLINE uint32_t R4l()                            { return zecLE32(iter::read<uint32_t>(_curr)); }
		ZEC_INLINE uint64_t R8l()                            { return zecLE64(iter::read<uint64_t>(_curr)); }
		ZEC_INLINE float    Rfl()                            { __detail__::__raw__::UF uf{.u=R4l()}; return uf.f; }
		ZEC_INLINE double   Rdl()                            { __detail__::__raw__::UD ud{.u=R8l()}; return ud.d; }

		ZEC_INLINE const void *      operator ()() const        { return _curr; }
		ZEC_INLINE operator          const ubyte * () const     { return _curr; }
		ZEC_INLINE const ubyte *     Origin() const             { return _start; }
		ZEC_INLINE ptrdiff_t         Offset() const             { return _curr - _start; }
		ZEC_INLINE const ubyte *     Skip(ptrdiff_t len = 0)    { const ubyte * from = _curr; _curr += len; return from; }

		ZEC_INLINE void Reset()                              { _curr = _start; }
		ZEC_INLINE void Reset(const void * s)                { _curr = _start = static_cast<const ubyte *>(s); }

	private:
		const ubyte * _curr = nullptr;
		const ubyte * _start = nullptr;
	};

}