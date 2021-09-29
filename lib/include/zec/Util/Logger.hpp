#pragma once
#include "../Common.hpp"
#include <filesystem>
#include <cstring>
#include <mutex>
#include <atomic>
#include <cinttypes>

ZEC_NS
{

	enum struct eLogLevel : int_fast32_t
	{	// !!! Note: implementation may use the value as index of output string,
		// so, ALWAYS increase values by ONE,
		Verbose = 0,
		Debug   = 1,
		Info    = 2,
		Error   = 3,
		Quiet   = 1024,
	};

	class xLogger : xAbstract
	{
	public:
		ZEC_API_MEMBER xLogger();
		ZEC_API_MEMBER ~xLogger();

		ZEC_API_MEMBER
		virtual void UpdateConfig(const char * aConfigData, size_t cConfigDataSize);
		virtual void SetLogLevel(eLogLevel ll) = 0;
		virtual void Log(eLogLevel ll, const char * fmt, ...) = 0;

		// helper functions
		ZEC_INLINE void UpdateConfig(const char * aConfigData) { UpdateConfig(aConfigData, strlen(aConfigData)); }

		template<typename ... Args>
		ZEC_INLINE void V(const char * fmt, Args&& ... args) {
			Log(eLogLevel::Verbose, fmt, std::forward<Args>(args)...);
		}

	#ifndef NDEBUG
		template<typename ... Args>
		ZEC_INLINE void D(const char * fmt, Args&& ... args) {
			Log(eLogLevel::Debug, fmt, std::forward<Args>(args)...);
		}
	#else
		template<typename ... Args>
		ZEC_INLINE void D(const char * fmt, Args&& ... args) {
			Pass();
		}
	#endif

		template<typename ... Args>
		ZEC_INLINE void I(const char * fmt, Args&& ... args) {
			Log(eLogLevel::Info, fmt, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		ZEC_INLINE void E(const char * fmt, Args&& ... args) {
			Log(eLogLevel::Error, fmt, std::forward<Args>(args)...);
		}
	};

	class xNonLogger final
	: public xLogger
	{
		void SetLogLevel(eLogLevel ll) override {}
		void Log(eLogLevel ll, const char * fmt, ...) override {};
	};

	class xSimpleLogger final
	: public xLogger
	{
	public:
		ZEC_API_MEMBER xSimpleLogger();
		ZEC_API_MEMBER ~xSimpleLogger();

		ZEC_API_MEMBER bool Init(const char * PathPtr = nullptr, bool AutoStdout = true);
		ZEC_API_MEMBER void Clean();
		ZEC_INLINE     bool IsStdout() const { return _LogFile == stdout; }

		ZEC_API_MEMBER void SetLogLevel(eLogLevel ll) override { _LogLevel = ll; }
		ZEC_API_MEMBER void Log(eLogLevel ll, const char * fmt, ...) override;

	private:
		std::filesystem::path        _LogFilename;
		std::mutex                   _SyncMutex;
		std::atomic<eLogLevel>       _LogLevel { eLogLevel::Debug };
		FILE *                       _LogFile = nullptr;
	};

	ZEC_API xNonLogger * const NonLoggerPtr;

}