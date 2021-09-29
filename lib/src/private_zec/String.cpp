#include <zec/String.hpp>
#include <zec/Byte.hpp>
#include <fstream>
#include <locale>
#include <cinttypes>

ZEC_NS
{

	static constexpr const char CTBL[256] =
	{
		0,0,0,0,0,0,0,0          ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,1,2,3,4,5,6,7         ,8,9,0,0,0,0,0,0
		,0,10,11,12,13,14,15,0   ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,10,11,12,13,14,15,0   ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
		,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
	};

	static constexpr const char HTBL[256][2] =
	{
		{'0','0'}, {'0','1'}, {'0','2'}, {'0','3'}, {'0','4'}, {'0','5'}, {'0','6'}, {'0','7'}, {'0','8'}, {'0','9'}, {'0','a'}, {'0','b'}, {'0','c'}, {'0','d'}, {'0','e'}, {'0','f'},
		{'1','0'}, {'1','1'}, {'1','2'}, {'1','3'}, {'1','4'}, {'1','5'}, {'1','6'}, {'1','7'}, {'1','8'}, {'1','9'}, {'1','a'}, {'1','b'}, {'1','c'}, {'1','d'}, {'1','e'}, {'1','f'},
		{'2','0'}, {'2','1'}, {'2','2'}, {'2','3'}, {'2','4'}, {'2','5'}, {'2','6'}, {'2','7'}, {'2','8'}, {'2','9'}, {'2','a'}, {'2','b'}, {'2','c'}, {'2','d'}, {'2','e'}, {'2','f'},
		{'3','0'}, {'3','1'}, {'3','2'}, {'3','3'}, {'3','4'}, {'3','5'}, {'3','6'}, {'3','7'}, {'3','8'}, {'3','9'}, {'3','a'}, {'3','b'}, {'3','c'}, {'3','d'}, {'3','e'}, {'3','f'},
		{'4','0'}, {'4','1'}, {'4','2'}, {'4','3'}, {'4','4'}, {'4','5'}, {'4','6'}, {'4','7'}, {'4','8'}, {'4','9'}, {'4','a'}, {'4','b'}, {'4','c'}, {'4','d'}, {'4','e'}, {'4','f'},
		{'5','0'}, {'5','1'}, {'5','2'}, {'5','3'}, {'5','4'}, {'5','5'}, {'5','6'}, {'5','7'}, {'5','8'}, {'5','9'}, {'5','a'}, {'5','b'}, {'5','c'}, {'5','d'}, {'5','e'}, {'5','f'},
		{'6','0'}, {'6','1'}, {'6','2'}, {'6','3'}, {'6','4'}, {'6','5'}, {'6','6'}, {'6','7'}, {'6','8'}, {'6','9'}, {'6','a'}, {'6','b'}, {'6','c'}, {'6','d'}, {'6','e'}, {'6','f'},
		{'7','0'}, {'7','1'}, {'7','2'}, {'7','3'}, {'7','4'}, {'7','5'}, {'7','6'}, {'7','7'}, {'7','8'}, {'7','9'}, {'7','a'}, {'7','b'}, {'7','c'}, {'7','d'}, {'7','e'}, {'7','f'},
		{'8','0'}, {'8','1'}, {'8','2'}, {'8','3'}, {'8','4'}, {'8','5'}, {'8','6'}, {'8','7'}, {'8','8'}, {'8','9'}, {'8','a'}, {'8','b'}, {'8','c'}, {'8','d'}, {'8','e'}, {'8','f'},
		{'9','0'}, {'9','1'}, {'9','2'}, {'9','3'}, {'9','4'}, {'9','5'}, {'9','6'}, {'9','7'}, {'9','8'}, {'9','9'}, {'9','a'}, {'9','b'}, {'9','c'}, {'9','d'}, {'9','e'}, {'9','f'},
		{'a','0'}, {'a','1'}, {'a','2'}, {'a','3'}, {'a','4'}, {'a','5'}, {'a','6'}, {'a','7'}, {'a','8'}, {'a','9'}, {'a','a'}, {'a','b'}, {'a','c'}, {'a','d'}, {'a','e'}, {'a','f'},
		{'b','0'}, {'b','1'}, {'b','2'}, {'b','3'}, {'b','4'}, {'b','5'}, {'b','6'}, {'b','7'}, {'b','8'}, {'b','9'}, {'b','a'}, {'b','b'}, {'b','c'}, {'b','d'}, {'b','e'}, {'b','f'},
		{'c','0'}, {'c','1'}, {'c','2'}, {'c','3'}, {'c','4'}, {'c','5'}, {'c','6'}, {'c','7'}, {'c','8'}, {'c','9'}, {'c','a'}, {'c','b'}, {'c','c'}, {'c','d'}, {'c','e'}, {'c','f'},
		{'d','0'}, {'d','1'}, {'d','2'}, {'d','3'}, {'d','4'}, {'d','5'}, {'d','6'}, {'d','7'}, {'d','8'}, {'d','9'}, {'d','a'}, {'d','b'}, {'d','c'}, {'d','d'}, {'d','e'}, {'d','f'},
		{'e','0'}, {'e','1'}, {'e','2'}, {'e','3'}, {'e','4'}, {'e','5'}, {'e','6'}, {'e','7'}, {'e','8'}, {'e','9'}, {'e','a'}, {'e','b'}, {'e','c'}, {'e','d'}, {'e','e'}, {'e','f'},
		{'f','0'}, {'f','1'}, {'f','2'}, {'f','3'}, {'f','4'}, {'f','5'}, {'f','6'}, {'f','7'}, {'f','8'}, {'f','9'}, {'f','a'}, {'f','b'}, {'f','c'}, {'f','d'}, {'f','e'}, {'f','f'},
	};

	static constexpr char HTBL_UPPER[256][2] =
	{
		{'0','0'}, {'0','1'}, {'0','2'}, {'0','3'}, {'0','4'}, {'0','5'}, {'0','6'}, {'0','7'}, {'0','8'}, {'0','9'}, {'0','A'}, {'0','B'}, {'0','C'}, {'0','D'}, {'0','E'}, {'0','F'},
		{'1','0'}, {'1','1'}, {'1','2'}, {'1','3'}, {'1','4'}, {'1','5'}, {'1','6'}, {'1','7'}, {'1','8'}, {'1','9'}, {'1','A'}, {'1','B'}, {'1','C'}, {'1','D'}, {'1','E'}, {'1','F'},
		{'2','0'}, {'2','1'}, {'2','2'}, {'2','3'}, {'2','4'}, {'2','5'}, {'2','6'}, {'2','7'}, {'2','8'}, {'2','9'}, {'2','A'}, {'2','B'}, {'2','C'}, {'2','D'}, {'2','E'}, {'2','F'},
		{'3','0'}, {'3','1'}, {'3','2'}, {'3','3'}, {'3','4'}, {'3','5'}, {'3','6'}, {'3','7'}, {'3','8'}, {'3','9'}, {'3','A'}, {'3','B'}, {'3','C'}, {'3','D'}, {'3','E'}, {'3','F'},
		{'4','0'}, {'4','1'}, {'4','2'}, {'4','3'}, {'4','4'}, {'4','5'}, {'4','6'}, {'4','7'}, {'4','8'}, {'4','9'}, {'4','A'}, {'4','B'}, {'4','C'}, {'4','D'}, {'4','E'}, {'4','F'},
		{'5','0'}, {'5','1'}, {'5','2'}, {'5','3'}, {'5','4'}, {'5','5'}, {'5','6'}, {'5','7'}, {'5','8'}, {'5','9'}, {'5','A'}, {'5','B'}, {'5','C'}, {'5','D'}, {'5','E'}, {'5','F'},
		{'6','0'}, {'6','1'}, {'6','2'}, {'6','3'}, {'6','4'}, {'6','5'}, {'6','6'}, {'6','7'}, {'6','8'}, {'6','9'}, {'6','A'}, {'6','B'}, {'6','C'}, {'6','D'}, {'6','E'}, {'6','F'},
		{'7','0'}, {'7','1'}, {'7','2'}, {'7','3'}, {'7','4'}, {'7','5'}, {'7','6'}, {'7','7'}, {'7','8'}, {'7','9'}, {'7','A'}, {'7','B'}, {'7','C'}, {'7','D'}, {'7','E'}, {'7','F'},
		{'8','0'}, {'8','1'}, {'8','2'}, {'8','3'}, {'8','4'}, {'8','5'}, {'8','6'}, {'8','7'}, {'8','8'}, {'8','9'}, {'8','A'}, {'8','B'}, {'8','C'}, {'8','D'}, {'8','E'}, {'8','F'},
		{'9','0'}, {'9','1'}, {'9','2'}, {'9','3'}, {'9','4'}, {'9','5'}, {'9','6'}, {'9','7'}, {'9','8'}, {'9','9'}, {'9','A'}, {'9','B'}, {'9','C'}, {'9','D'}, {'9','E'}, {'9','F'},
		{'A','0'}, {'A','1'}, {'A','2'}, {'A','3'}, {'A','4'}, {'A','5'}, {'A','6'}, {'A','7'}, {'A','8'}, {'A','9'}, {'A','A'}, {'A','B'}, {'A','C'}, {'A','D'}, {'A','E'}, {'A','F'},
		{'B','0'}, {'B','1'}, {'B','2'}, {'B','3'}, {'B','4'}, {'B','5'}, {'B','6'}, {'B','7'}, {'B','8'}, {'B','9'}, {'B','A'}, {'B','B'}, {'B','C'}, {'B','D'}, {'B','E'}, {'B','F'},
		{'C','0'}, {'C','1'}, {'C','2'}, {'C','3'}, {'C','4'}, {'C','5'}, {'C','6'}, {'C','7'}, {'C','8'}, {'C','9'}, {'C','A'}, {'C','B'}, {'C','C'}, {'C','D'}, {'C','E'}, {'C','F'},
		{'D','0'}, {'D','1'}, {'D','2'}, {'D','3'}, {'D','4'}, {'D','5'}, {'D','6'}, {'D','7'}, {'D','8'}, {'D','9'}, {'D','A'}, {'D','B'}, {'D','C'}, {'D','D'}, {'D','E'}, {'D','F'},
		{'E','0'}, {'E','1'}, {'E','2'}, {'E','3'}, {'E','4'}, {'E','5'}, {'E','6'}, {'E','7'}, {'E','8'}, {'E','9'}, {'E','A'}, {'E','B'}, {'E','C'}, {'E','D'}, {'E','E'}, {'E','F'},
		{'F','0'}, {'F','1'}, {'F','2'}, {'F','3'}, {'F','4'}, {'F','5'}, {'F','6'}, {'F','7'}, {'F','8'}, {'F','9'}, {'F','A'}, {'F','B'}, {'F','C'}, {'F','D'}, {'F','E'}, {'F','F'},
	};

	using Hex2 = char[2];

	// ZEC_STATIC_INLINE constexpr const Hex2 & StrToHexLower(char c) { return HTBL[static_cast<unsigned char>(c)]; }
	// ZEC_STATIC_INLINE constexpr const Hex2 & StrToHex(char c) { return HTBL_UPPER[static_cast<unsigned char>(c)]; }
	// ZEC_STATIC_INLINE constexpr char x2c(const Hex2 &pc2) {
	// 	auto uc0 = static_cast<unsigned char>(pc2[0]);
	// 	auto uc1 = static_cast<unsigned char>(pc2[1]);
	// 	return static_cast<char>((CTBL[uc0] << 4) + CTBL[uc1]);
	// }

	std::vector<std::string> Split(const std::string_view & s, const char * d, size_t l)
	{
		assert(l);
		std::vector<std::string> r;
		std::string::size_type pos = 0;
		std::string::size_type end = 0;
		std::string::size_type len = static_cast<std::string::size_type>(l);

		while(true) {
			end = s.find(d, pos, len);

			if (end == std::string::npos) {
				r.push_back(std::string{s.substr(pos, end)});
				break;
			}
			else {
				r.push_back(std::string{s.substr(pos, end - pos)});
				pos = end + len;
			}
		}
		return r;
	}

	std::string Trim(const std::string_view & s)
	{
		int code = 0;
		auto it = s.begin();
		for(; it != s.end(); ++it) {
			code = (unsigned char)(*it);
			if (!isspace(code)) {
				break;
			}
		}
		std::string_view tleft = s.substr(it - s.begin());
		if (tleft.length() == 0) {
			return std::string{tleft};
		}

		const char * data = tleft.data();
		size_t idx = tleft.length() - 1;
		while (idx != 0) {
			code = (unsigned char)data[idx];
			if (isspace(code)) {
				--idx;
				continue;
			}
			break;
		}
		return std::string{tleft.substr(0, idx+1)};
	}

	void Reverse(void * str_, size_t len)
	{
		if (!len)
			return ;

		char t;
		char * str = static_cast<char*>(str_);
		char * last = str + len - 1;

		do {
			t = *str;
			*str = *last;
			*last = t;
		} while(++str < --last);
	}

	void CStrNCpy(char * dst, size_t n, const char * src) {
		if (n > 0) {
			strncpy(dst, src, n);
			dst[n - 1] = '\0';
		}
	}

	static std::string __show__(const void * buffer_, size_t len, const char (&hexfmt)[5], bool header)
	{
		if (!len) {
			return "<empty>";
		}
		const char * buffer = static_cast<const char*>(buffer_);
		std::string h;
		std::string p;
		char lineno[32];
		char ch [32];
		char bh [128];
		char bp [128];
		if (header) {
		//	h.append("00000000  0001 0203 0405 0607 0809 0a0b 0c0d 0e0f : ................")
			h.append("+-Line-+  +-----------------Hex-----------------+   +-----Char-----+\n");
		}
		h.append("00000000  ");
		for (size_t oi = 0 ; oi < len ; ++oi) {
			char c = buffer[oi];
			char cs = (isprint(c) && c != ' ') ? c : '.' ;
			sprintf(ch, hexfmt, (int)(unsigned char)c);
			sprintf(bh, ((oi % 2) ? "%s ": "%s"), ch);
			sprintf(bp, "%c", cs);
			if (oi) {
				if(oi%16 == 0) {
					h.append("  ");
					h.append(p);
					h.append("\n");
					sprintf(lineno, "%08" PRIx32 "  ", (uint32_t)((oi / 16)));
					h.append(lineno, 10);
					p.clear();
				}
			}
			h.append(bh);
			p.append(bp);
		}
		size_t f = 50 - h.length() % 69;
		h.append(
				"                "
				"                "
				"                "
				"                "
				, f);
		h.append("  ");
		h.append(p);
		return h;
	}

	std::string HexShowLower(const void * buffer_, size_t len, bool header)
	{
		return __show__(buffer_, len, "%02x", header);
	}

	std::string HexShow(const void * buffer_, size_t len, bool header)
	{
		return __show__(buffer_, len, "%02X", header);
	}


	std::string HexToStr(const void * d_, size_t len)
	{
		const char * d = static_cast<const char*>(d_);
		size_t mask = 1;
		len &= ~mask;
		std::string ret;
		ret.reserve(len / 2);
		for(size_t i = 0 ; i < len ; i += 2) {
			unsigned char uc0 = static_cast<unsigned char>(d[i]);
			unsigned char uc1 = static_cast<unsigned char>(d[i+1]);
			char off = (CTBL[uc0] << 4) + CTBL[uc1];
			ret.append(&off, 1);
		}
		return ret;
	}

	void HexToStr(void * dst, const void * src, size_t len)
	{
		const unsigned char * s = static_cast<const unsigned char*>(src);
		size_t mask = 1;
		len &= ~mask;
		unsigned char uc0, uc1;
		xStreamWriter w(dst);
		for(size_t i = 0 ; i < len ; i += 2) {
			uc0 = s[i];
			uc1 = s[i+1];
			w.W((CTBL[uc0] << 4) + CTBL[uc1]);
		}
	}

	void StrToHexLower(void * dst, const void * str, size_t len)
	{
		xStreamWriter w(dst);
		const unsigned char * curr = static_cast<const unsigned char *>(str);
		const unsigned char * end = curr + len;
		unsigned char hoff;
		for (; curr < end ; ++curr) {
			hoff = *curr ;
			w.W(HTBL [hoff][0]);
			w.W(HTBL [hoff][1]);
		}
	}

	std::string StrToHexLower(const void * str, size_t len)
	{
		char buffer[256];
		const size_t pitch = 128;
		size_t off = 0;
		size_t lines = len / pitch;
		size_t lastOff = len % pitch;
		const char * curr = static_cast<const char*>(str);
		std::string ret;
		ret.reserve(len * 2);

		xStreamWriter w(buffer);
		unsigned char hoff;
		for (off = 0 ; off < lines; ++off) {
			for (size_t i = 0 ; i < pitch; ++i) {
				hoff = (unsigned char &)*curr ;
				w.W(HTBL [hoff][0]);
				w.W(HTBL [hoff][1]);
				++curr;
			}
			ret.append(buffer, sizeof(buffer));
			w.Reset();
		}

		for (size_t i = 0 ; i < lastOff; ++i) {
			hoff = (unsigned char&)*curr ;
			w.W(HTBL [hoff][0]);
			w.W(HTBL [hoff][1]);
			++curr;
		}
		ret.append(buffer, lastOff * 2);
		return ret;
	}


	void StrToHex(void * dst, const void * str, size_t len)
	{
		xStreamWriter w(dst);
		const unsigned char * curr = static_cast<const unsigned char *>(str);
		const unsigned char * end = curr + len;
		unsigned char hoff;
		for (; curr < end ; ++curr) {
			hoff = *curr ;
			w.W(HTBL_UPPER [hoff][0]);
			w.W(HTBL_UPPER [hoff][1]);
		}
	}

	std::string StrToHex(const void * str, size_t len)
	{
		char buffer[256];
		const size_t pitch = 128;
		size_t off = 0;
		size_t lines = len / pitch;
		size_t lastOff = len % pitch;
		const char * curr = static_cast<const char*>(str);
		std::string ret;
		ret.reserve(len * 2);

		xStreamWriter w(buffer);
		unsigned char hoff;
		for (off = 0 ; off < lines; ++off) {
			for (size_t i = 0 ; i < pitch; ++i) {
				hoff = (unsigned char &)*curr ;
				w.W(HTBL_UPPER [hoff][0]);
				w.W(HTBL_UPPER [hoff][1]);
				++curr;
			}
			ret.append(buffer, sizeof(buffer));
			w.Reset();
		}

		for (size_t i = 0 ; i < lastOff; ++i) {
			hoff = (unsigned char&)*curr ;
			w.W(HTBL_UPPER [hoff][0]);
			w.W(HTBL_UPPER [hoff][1]);
			++curr;
		}
		ret.append(buffer, lastOff * 2);
		return ret;
	}

	std::string StrToHexLower(const char * str)
	{
		return StrToHexLower(str, strlen(str));
	}

	std::string StrToHex(const char * str)
	{
		return StrToHex(str, strlen(str));
	}

	#ifndef __cpp_char8_t
	using char8_t = char;
	#endif

	namespace {
		using Cvt = std::codecvt<char32_t, char8_t, std::mbstate_t>;
	}

	std::u32string ToUtf32(const std::string_view & U8String)
	{
		std::u32string Output;
		auto Length = U8String.size();
		Output.resize(Length);

		auto & Facet = std::use_facet<Cvt>(std::locale::classic());
		const char8_t *   FromNext = reinterpret_cast<const char8_t *>(U8String.data());
		char32_t *        ToStart = Output.data();
		char32_t *        ToNext = ToStart;
		std::mbstate_t    State {};

		Facet.in(State, FromNext, FromNext + Length, FromNext,
			ToStart, ToStart + Length, ToNext);
		Output.resize(ToNext - ToStart);
		return Output;
	}

	std::string ToUtf8(const std::u32string_view & U32String)
	{
		std::string Output;
		auto Length = U32String.size();
		Output.resize(Length * 3);

		auto & Facet = std::use_facet<Cvt>(std::locale::classic());
		const char32_t *  FromNext = U32String.data();
		char8_t *         ToStart = reinterpret_cast<char8_t*>(Output.data());
		char8_t *         ToNext = ToStart;
		std::mbstate_t    State {};

		Facet.out(State, FromNext, FromNext + Length, FromNext,
			ToStart, ToStart + Output.size(), ToNext);
		Output.resize(ToNext - ToStart);
		return Output;
	}

	xOptional<std::string> FileToStr(const char * filename)
	{
		std::ifstream File(filename, std::ios::binary | std::ios::ate);
		if (!File) {
			return {};
		}

		std::streamsize Size = File.tellg();
		File.seekg(0, std::ios::beg);

		std::string Output;
		Output.resize(Size);
		File.read(Output.data(), Size);
		return Output;
	}

}
