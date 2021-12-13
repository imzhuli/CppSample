#include <zec/Util/Logger.hpp>
#include <zec/Byte.hpp>
#include <zec/CArray.hpp>
#include <cstdio>
#include <ctime>
#include <string>
#include <thread>
#include <cstring>
#include <cstdarg>
#include <string>
#include <algorithm>

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
	{ assert(!_LogFile); }

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
		} else {
			_LogFile = nullptr;
		}
		SetLogLevel( eLogLevel::Debug );
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


	xMemoryLogger::xMemoryLogger()
	{}

	xMemoryLogger::~xMemoryLogger()
	{ assert(!_LogBufferPtr); }

	bool xMemoryLogger::Init(size32_t MaxLineNumber, size32_t MaxLineSize)
	{
		assert(MaxLineNumber > 1);
		assert(MaxLineSize > 1);
		assert(!_LogBufferPtr);

		++MaxLineNumber;
		_LineSize = MaxLineSize;
		_RealLineSize = LineLeadBufferSize + _LineSize + ExtraSize;
		size_t TotalBufferSize = MaxLineNumber * _RealLineSize;
		_LogBufferPtr = (char*)malloc(TotalBufferSize);
		if (!_LogBufferPtr) {
			Reset(_LineSize);
			Reset(_RealLineSize);
			return false;
		}
		_CurrentLineIndex = 0;
		_LineNumber = MaxLineNumber;
		memset(_LogBufferPtr, 0, TotalBufferSize);
		return true;
	}

	void xMemoryLogger::Clean()
	{
		assert(_LogBufferPtr);
		free(Steal(_LogBufferPtr));
		Reset(_LineSize);
		Reset(_RealLineSize);
		Reset(_LineNumber);
		Reset(_CurrentLineIndex);
		SetLogLevel( eLogLevel::Debug );
	}

	void xMemoryLogger::Log(eLogLevel ll, const char * fmt, ...)
	{
		std::tm brokenTime;
		std::time_t now = std::time(nullptr);
#ifdef _MSC_VER
		localtime_s(&brokenTime, &now);
#else
		localtime_r(&now, &brokenTime);
#endif

		va_list vaList;
		va_start(vaList, fmt);

		char LineLead[LineLeadBufferSize];
		int LineLeadSize = snprintf(LineLead, SafeLength(LineLead), "%c<%016zx>%02d%02d%02d:%02d%02d%02d ", gcHint[static_cast<size_t>(ll)],
			std::hash<std::thread::id>{}(std::this_thread::get_id()),
			brokenTime.tm_year + 1900 - 2000, brokenTime.tm_mon + 1, brokenTime.tm_mday,
			brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
		);

		do { // synchronized block
			auto Guard = xSpinlockGuard { _Spinlock };

			assert(_LogBufferPtr);
			char * LineStart = _LogBufferPtr + _CurrentLineIndex * _RealLineSize;
			xStreamWriter S(LineStart);
			S.W(LineLead, LineLeadSize);
			int LogLength = vsnprintf((char*)S(), _LineSize, fmt, vaList);
			if (LogLength > 0) {
				S.Skip(std::min((size_t)LogLength, _LineSize - 1));
			}
			S.W('\n');
			S.W('\0');
			// printf("---offset:%d/%d/%d---", (int)S.Offset(), (int)LogLength, (int)_LineSize);

			++_CurrentLineIndex;
			_CurrentLineIndex %= _LineNumber;
		} while(false);

		va_end(vaList);
		return;
	}

	void xMemoryLogger::Output(FILE * fp)
	{
		if (!fp) {
			return;
		}
		auto Guard = xSpinlockGuard { _Spinlock };

		size_t StartIndex = _CurrentLineIndex + 1;
		while(StartIndex != _CurrentLineIndex) {
			auto LineStart = _LogBufferPtr + StartIndex * _RealLineSize;
			if (*LineStart) {
				fprintf(fp, "%s", LineStart);
			}
			++StartIndex;
			StartIndex %= _LineNumber;
		}
	}

}
