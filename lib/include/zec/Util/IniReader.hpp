#pragma once
#include "../Common.hpp"

ZEC_NS
{

	class xIniReader
	{
	public:
		ZEC_API_MEMBER xIniReader(const char * filename);
		ZEC_API_MEMBER xIniReader(xIniReader &&) = delete;
		ZEC_API_MEMBER ~xIniReader();

		ZEC_INLINE operator bool () const { return _pContent; }

		ZEC_API_MEMBER const char * Get(const char * key) const;
		ZEC_API_MEMBER bool         GetBool(const char * key, bool defaultValue = false) const;
		ZEC_API_MEMBER int64_t      GetInt64(const char * key, int64_t defaultValue = 0) const;

	private:
		struct __IniContent__ * _pContent {};
	};

}