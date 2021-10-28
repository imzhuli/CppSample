#pragma once
#include <zec/Common.hpp>
#include <string>

ZEC_NS
{

	class xAssetPath
	{
	public:
		ZEC_API_MEMBER xAssetPath(const char * Path = nullptr);

		ZEC_INLINE const char * c_str() const { return Get().c_str(); }
		ZEC_INLINE const std::string & str() const { return Get(); }

		ZEC_INLINE operator const char * () const { return c_str(); }
		ZEC_INLINE const std::string & Get() const { return _FixedPath; }

	public:
		ZEC_API_STATIC_MEMBER void ChangeRoot(const char * RootPath);
		ZEC_API_STATIC_MEMBER bool CreateDirectory(const char * AbsolutePath);
		ZEC_API_STATIC_MEMBER bool Remove(const char * AbsolutePath);
		ZEC_API_STATIC_MEMBER bool RemoveAll(const char * AbsolutePath);

	private:
		std::string _FixedPath;
	};

}
