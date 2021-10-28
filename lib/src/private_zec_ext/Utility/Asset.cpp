#include <zec_ext/Utility/Asset.hpp>
#include <zec/Util/Thread.hpp>
#include <filesystem>
#include <string>
#include <mutex>
ZEC_NS
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

}
