#pragma once
#include <xel/Common.hpp>
#include <filesystem>
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

	class xTempPath
    {
    public:
        X_API_MEMBER xTempPath();
        X_API_MEMBER ~xTempPath();
        xTempPath(const xTempPath &) = delete;

        X_INLINE operator bool () const { return _Created; }
        X_INLINE std::string ToString () const { return _TempPath.string(); }
        X_INLINE const std::filesystem::path & Get() const { return _TempPath; }

    private:
        bool                  _Created = false;
        std::filesystem::path _TempPath;
    };


}
