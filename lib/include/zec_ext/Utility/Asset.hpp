#pragma once
#include <zec/Common.hpp>
#include <string>

X_NS
{

	class xAssetPath
	{
	public:
		X_API_MEMBER xAssetPath(const char * Path = nullptr);

		X_INLINE const char * c_str() const { return Get().c_str(); }
		X_INLINE const std::string & str() const { return Get(); }

		X_INLINE operator const char * () const { return c_str(); }
		X_INLINE const std::string & Get() const { return _FixedPath; }

	public:
		X_API_STATIC_MEMBER void ChangeRoot(const char * RootPath);
		X_API_STATIC_MEMBER bool CreateDirectory(const char * AbsolutePath);
		X_API_STATIC_MEMBER bool Remove(const char * AbsolutePath);
		X_API_STATIC_MEMBER bool RemoveAll(const char * AbsolutePath);

	private:
		std::string _FixedPath;
	};

}
