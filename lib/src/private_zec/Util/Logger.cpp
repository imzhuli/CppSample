#include <zec/Util/Logger.hpp>
#include <zec/CArray.hpp>
#include <cstdio>
#include <ctime>
#include <string>
#include <thread>
#include <cstring>
#include <cstdarg>
#include <string>

ZEC_NS
{

	static constexpr auto gcHint = xCArray {
		xType<const char>,
		'V',
		'D',
		'I',
		'E',
	};

	xLogger::xLogger()
	{}

	xLogger::~xLogger()
	{}

	void xLogger::UpdateConfig(const char * aConfigData, size_t cConfigDataSize)
	{}

	xSimpleLogger::xSimpleLogger()
	{}

	xSimpleLogger::~xSimpleLogger()
	{}

	bool xSimpleLogger::Init(const char * PathPtr, bool AutoStdout)
	{
		if (PathPtr) {
			_LogFilename = PathPtr;
		}
		else {
			return (AutoStdout ? (_LogFile = stdout) : (_LogFile = nullptr));
		}
		_LogFile = fopen(_LogFilename.string().c_str(), "at");
		return _LogFile != nullptr;
	}

	void xSimpleLogger::Clean()
	{
		if (_LogFile != stdout) {
			fclose(Steal(_LogFile));
		}
	}

	void xSimpleLogger::Log(eLogLevel ll, const char * fmt, ...) {
		if (ll < _LogLevel || !_LogFile) {
			return;
		}

		std::tm brokenTime;
		std::time_t now = std::time(nullptr);
#ifdef _MSC_VER
		localtime_s(&brokenTime, &now);
#else
		localtime_r(&now, &brokenTime);
#endif
		std::hash<std::thread::id> hasher;

		va_list vaList;
		va_start(vaList, fmt);

		do { // synchronized block
			auto guard = std::lock_guard { _SyncMutex };
			fprintf(_LogFile, "%c<%016zx>%02d%02d%02d:%02d%02d%02d ", gcHint[static_cast<size_t>(ll)],
				hasher(std::this_thread::get_id()),
				brokenTime.tm_year + 1900 - 2000, brokenTime.tm_mon + 1, brokenTime.tm_mday,
				brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
			);
			vfprintf(_LogFile, fmt, vaList);
			fputc('\n', _LogFile);
		} while(false);
		fflush(_LogFile);

		va_end(vaList);
		return;
	}

	static xNonLogger      NonLogger{};
	xNonLogger *  const    NonLoggerPtr = &NonLogger;

}
