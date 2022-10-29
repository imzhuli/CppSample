#pragma once
#include "./Common.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <cstring>

X_NS
{
    using namespace std::literals::string_view_literals;

	X_API               std::vector<std::string> Split(const std::string_view & s, const char * d, size_t len);
	X_STATIC_INLINE     std::vector<std::string> Split(const std::string_view & s, const char * d) { return Split(s, d, strlen(d)); }

	X_API std::string Trim(const std::string_view & s);
	X_API void Reverse(void * str, size_t len);
	X_API void CStrNCpy(char * dst, size_t n, const char * src);
	template<size_t N>
	X_STATIC_INLINE void CStrNCpy(char (&dsrArr)[N], const char *src) {
		CStrNCpy((char *)dsrArr, N, src);
	}

	X_API std::string HexShowLower(const void * buffer, size_t len, bool header = false);
	X_API std::string HexShow(const void * buffer, size_t len, bool header = false);

	X_API void HexToStr(void * dst, const void * str, size_t len);
	X_API std::string HexToStr(const void * str, size_t len);

	X_API void StrToHexLower(void * dst, const void * str, size_t len);
	X_API std::string StrToHexLower(const void * str, size_t len);
	X_API std::string StrToHexLower(const char * str);
	X_INLINE std::string StrToHexLower(const std::string_view & sv) {
		return StrToHexLower(sv.data(), sv.length());
	}

	X_API void StrToHex(void * dst, const void * str, size_t len);
	X_API std::string StrToHex(const void * str, size_t len);
	X_API std::string StrToHex(const char * str);
	X_INLINE std::string StrToHex(const std::string_view & sv) {
		return StrToHex(sv.data(), sv.length());
	}

	X_API std::u32string ToUtf32(const std::string_view & U8String);
	X_API std::string ToUtf8(const std::u32string_view & U32String);
	X_API xOptional<std::string> FileToStr(const char * filename);
	X_INLINE xOptional<std::string> FileToStr(const std::string & filename)  {
		return FileToStr(filename.c_str());
	}

}
