#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	//define something for Windows (32-bit and 64-bit, this part is common)
	#pragma warning (disable:4180)
	#pragma warning (disable:4200)
	#pragma warning (disable:4819)
	#define ZEC_SYSTEM_WINDOWS
	#ifdef _WIN64
		//define something for Windows (64-bit only)
	  	#define ZEC_SYSTEM_WIN64
	#else
		//define something for Windows (32-bit only)
		#define ZEC_SYSTEM_WIN32
	#endif
#elif __APPLE__
	#define ZEC_SYSTEM_APPLE
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		#define ZEC_SYSTEM_IPHONE_SIMULATOR
	#elif TARGET_OS_IPHONE
		// iOS device
		#define ZEC_SYSTEM_IPHONE
	#elif TARGET_OS_MAC
		// Other kinds of Mac OS
		#define ZEC_SYSTEM_MACOS
	#else
	#	error "Unknown Apple platform"
	#endif
#elif defined(__ANDROID_API__)
	#define ZEC_SYSTEM_ANDROID
#elif __linux__
	// linux
	#define ZEC_SYSTEM_LINUX
	#ifdef __FreeBSD__
		#define ZEC_SYSTEM_FREEBSD
	#endif
#elif __unix__ // all unices not caught above
	// Unix
	#error "unsupported unix"
#elif defined(_POSIX_VERSION)
	// POSIX
	#error "unsupported posix"
#else
	#error "Unknown system type"
#endif

#if defined(ZEC_SYSTEM_WINDOWS) || defined(ZEC_SYSTEM_LINUX) || defined(ZEC_SYSTEM_MACOS)
	#define ZEC_SYSTEM_DESKTOP
#endif

#if defined(ZEC_SYSTEM_IPHONE_SIMULATOR) || defined(ZEC_SYSTEM_IPHONE)
	#define ZEC_SYSTEM_IOS
#endif

#if defined(_MSC_VER)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#define ZEC_INLINE                       __forceinline
	#define ZEC_STATIC_INLINE                static __forceinline

	#define ZEC_PRIVATE                      extern
	#define ZEC_PRIVATE_MEMBER
	#define ZEC_PRIVATE_STATIC_MEMBER        static
	#define ZEC_PRIVATE_CONSTEXPR            constexpr
	#define ZEC_PRIVATE_STATIC_CONSTEXPR     static constexpr
	#define ZEC_PRIVATE_INLINE               __forceinline

	#define ZEC_EXPORT                       __declspec(dllexport) extern
	#define ZEC_EXPORT_MEMBER                __declspec(dllexport)
	#define ZEC_EXPORT_STATIC_MEMBER         __declspec(dllexport) static
	#define ZEC_IMPORT                       __declspec(dllimport) extern
	#define ZEC_IMPORT_MEMBER                __declspec(dllimport)
	#define ZEC_IMPORT_STATIC_MEMBER         __declspec(dllimport) static
#elif defined(__clang__) || defined(__GNUC__)
	#define ZEC_INLINE                       __attribute__((always_inline)) inline
	#define ZEC_STATIC_INLINE                __attribute__((always_inline)) static inline

	#define ZEC_PRIVATE                      __attribute__((__visibility__("hidden"))) extern
	#define ZEC_PRIVATE_MEMBER               __attribute__((__visibility__("hidden")))
	#define ZEC_PRIVATE_STATIC_MEMBER        __attribute__((__visibility__("hidden"))) static
	#define ZEC_PRIVATE_CONSTEXPR            __attribute__((__visibility__("hidden"))) constexpr
	#define ZEC_PRIVATE_STATIC_CONSTEXPR     __attribute__((__visibility__("hidden"))) static constexpr
	#define ZEC_PRIVATE_INLINE               __attribute__((always_inline)) __attribute__((__visibility__("hidden"))) inline

	#define ZEC_EXPORT                       __attribute__((__visibility__("default"))) extern
	#define ZEC_EXPORT_MEMBER                __attribute__((__visibility__("default")))
	#define ZEC_EXPORT_STATIC_MEMBER         __attribute__((__visibility__("default"))) static
	#define ZEC_IMPORT                       extern
	#define ZEC_IMPORT_MEMBER
	#define ZEC_IMPORT_STATIC_MEMBER         static
#else
	#error "Unsupported compiler"
#endif

#define ZEC_EXTERN             extern
#define ZEC_MEMBER
#define ZEC_STATIC_MEMBER      static
#if defined(ZEC_OPTION_STATIC)
	#if defined(ZEC_OPTION_EXPORT_API)
		#error ZEC_OPTION_STATIC is used with ZEC_OPTION_EXPORT_API
	#endif
	#define ZEC_API                      ZEC_EXTERN
	#define ZEC_API_MEMBER               ZEC_MEMBER
	#define ZEC_API_STATIC_MEMBER        ZEC_STATIC_MEMBER
#else
	#if defined(ZEC_OPTION_EXPORT_API)
		#define ZEC_API                  ZEC_EXPORT
		#define ZEC_API_MEMBER           ZEC_EXPORT_MEMBER
		#define ZEC_API_STATIC_MEMBER    ZEC_EXPORT_STATIC_MEMBER
	#else
		#define ZEC_API                  ZEC_IMPORT
		#define ZEC_API_MEMBER           ZEC_IMPORT_MEMBER
		#define ZEC_API_STATIC_MEMBER    ZEC_IMPORT_STATIC_MEMBER
	#endif
#endif

#ifndef __cplusplus
#define ZEC_CNAME
#define ZEC_CNAME_BEGIN
#define ZEC_CNAME_END
#else
#define ZEC_CNAME extern "C"
#define ZEC_CNAME_BEGIN extern "C" {
#define ZEC_CNAME_END }
#endif

#define ZEC_MAKE_STRING(s) #s
#define ZEC_EXPAND_STRING(s) ZEC_MAKE_STRING(s)
#define ZEC_EXPECT(x) do { bool test=(bool)(x); if(!test) { throw #x; } } while(0)

#if defined(_MSC_VER)
#	define ZecAlignedAlloc                      _aligned_malloc
#	define ZecAlignedFree                       _aligned_free
#   define ZecLocalTime(Timestamp, ST)          localtime_s((ST), (Timestamp));
#else
#	define ZecAlignedAlloc(size, alignment)     aligned_alloc(alignment, size)
#	define ZecAlignedFree                       free
#   define ZecLocalTime(Timestamp, ST)          localtime_r((Timestamp), (ST));
#endif

#if defined(ZEC_SYSTEM_ANDROID)
#	define _ZEC_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName) \
		Java_ ## PackageName ## _ ## ClassFuncName
#	define ZEC_ANDROID_API__FUNC_NAME(PackageName, ClassFuncName) \
		_ZEC_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName)

#if defined(ANDROID_PACKAGE_CNAME)
#	define _ZEC_ANDROID_API(ClassFuncName, ReturnType) \
		ZEC_CNAME JNIEXPORT ReturnType JNICALL ZEC_ANDROID_API__FUNC_NAME(ANDROID_PACKAGE_CNAME, ClassFuncName)
#	define ZEC_ANDROID_API(ClassName, FuncName, ReturnType) _ZEC_ANDROID_API(ClassName ## _ ## FuncName, ReturnType)
#	endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
