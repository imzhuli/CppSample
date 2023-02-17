#include <xel_ext/Util/FileSystem.hpp>
#include <xel_ext/Util/Uuid.hpp>
#include <xel/Util/Thread.hpp>
#include <xel/String.hpp>
#include <filesystem>
#include <string>
#include <mutex>

X_NS
{
    static xSpinlock RootPathLock;
    static std::filesystem::path RootPath;

    void xAssetPath::ChangeRoot(const char * RootPathStr) {
        xSpinlockGuard Guard{RootPathLock};
        RootPath = RootPathStr;
    }

    xAssetPath::xAssetPath(const char * Path) {
        xSpinlockGuard Guard{RootPathLock};
        auto CompletePath = Path ? (RootPath / std::filesystem::path(std::string(Path))) : RootPath;
		_FixedPath = CompletePath.string();
    }

	bool xAssetPath::CreateDirectory(const char * AbsolutePath)
	{
		return std::filesystem::create_directory(AbsolutePath);
	}

	bool xAssetPath::Remove(const char * AbsolutePath)
	{
		return std::filesystem::remove(AbsolutePath);
	}

	bool xAssetPath::RemoveAll(const char * AbsolutePath)
	{
		return std::filesystem::remove_all(AbsolutePath);
	}

    xTempPath::xTempPath()
    {
        xUuid Uuid;
		Uuid.Generate();

        auto Error = std::error_code();
        _TempPath = std::filesystem::temp_directory_path(Error);

        if (Error) {
            return;
        }

        _TempPath = _TempPath / StrToHex(Uuid.GetData(), Uuid.GetSize());
        if (!create_directories(_TempPath, Error)) {
            return;
        }

        _Created = true;
    }

    xTempPath::~xTempPath()
    {
        if (_Created) {
            remove_all(_TempPath, X2Ref(std::error_code()));
        }
    }

}
