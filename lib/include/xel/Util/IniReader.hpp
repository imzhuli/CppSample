#pragma once
#include "../Common.hpp"

X_NS
{

	class xIniReader
	{
	public:
		X_API_MEMBER xIniReader(const char * filename);
		X_API_MEMBER xIniReader(xIniReader &&) = delete;
		X_API_MEMBER ~xIniReader();

		X_INLINE operator bool () const { return _pContent; }

		X_API_MEMBER const char * Get(const char * key, const char * defaultValue = nullptr) const;
		X_API_MEMBER bool         GetBool(const char * key,     bool defaultValue = false) const;
		X_API_MEMBER int64_t      GetInt64(const char * key, int64_t defaultValue = 0) const;

	private:
		struct __IniContent__ * _pContent {};
	};

}