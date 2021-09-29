#include <zec/Common.hpp>

ZEC_NS {

	static_assert(sizeof(ubyte) == sizeof(byte) && sizeof(uint8_t) == sizeof(byte));
	static_assert(sizeof(intptr_t) == sizeof(void*));
	static_assert(sizeof(xVariable) == sizeof(uint64_t));

	/***
	 * According to C++ standard,
	 * char32_t is of size of uint_least32_t, but a distinct type
	 * but it would a big wast of apce if char32_t size is larger than sizeof(uint32_t)
	 * we assume and check that chat32_t is of same sizeof(uing32_t)
	 *
	 * it will be complex to have another defination of char32_t in such cases,
	 * like enum my_char32_t : uint32_t {};
	 * but it still may not be compat with std::string32_view.
	 * so, such implementation would not be supported unless it becomes a common thing on most platforms
	 * */
	static_assert(sizeof(char32_t) == sizeof(uint32_t));

}
