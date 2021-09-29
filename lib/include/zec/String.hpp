#pragma once
#include "./Common.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <cstring>

ZEC_NS
{

	ZEC_API               std::vector<std::string> Split(const std::string_view & s, const char * d, size_t len);
	ZEC_STATIC_INLINE     std::vector<std::string> Split(const std::string_view & s, const char * d) { return Split(s, d, strlen(d)); }

	ZEC_API std::string Trim(const std::string_view & s);
	ZEC_API void Reverse(void * str, size_t len);
	ZEC_API void CStrNCpy(char * dst, size_t n, const char * src);
	template<size_t N>
	ZEC_STATIC_INLINE void CStrNCpy(char (&dsrArr)[N], const char *src) {
		CStrNCpy((char *)dsrArr, N, src);
	}

	ZEC_API std::string HexShowLower(const void * buffer, size_t len, bool header = false);
	ZEC_API std::string HexShow(const void * buffer, size_t len, bool header = false);

	ZEC_API void HexToStr(void * dst, const void * str, size_t len);
	ZEC_API std::string HexToStr(const void * str, size_t len);

	ZEC_API void StrToHexLower(void * dst, const void * str, size_t len);
	ZEC_API std::string StrToHexLower(const void * str, size_t len);
	ZEC_API std::string StrToHexLower(const char * str);
	ZEC_INLINE std::string StrToHexLower(const std::string_view & sv) {
		return StrToHexLower(sv.data(), sv.length());
	}

	ZEC_API void StrToHex(void * dst, const void * str, size_t len);
	ZEC_API std::string StrToHex(const void * str, size_t len);
	ZEC_API std::string StrToHex(const char * str);
	ZEC_INLINE std::string StrToHex(const std::string_view & sv) {
		return StrToHex(sv.data(), sv.length());
	}

	ZEC_API std::u32string ToUtf32(const std::string_view & U8String);
	ZEC_API std::string ToUtf8(const std::u32string_view & U32String);
	ZEC_API xOptional<std::string> FileToStr(const char * filename);
	ZEC_INLINE xOptional<std::string> FileToStr(const std::string & filename)  {
		return FileToStr(filename.c_str());
	}

}
