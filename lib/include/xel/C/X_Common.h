#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	//define something for Windows (32-bit and 64-bit, this part is common)
	#pragma warning (disable:4180)
	#pragma warning (disable:4200)
	#pragma warning (disable:4819)
	#define X_SYSTEM_WINDOWS
	#ifdef _WIN64
		//define something for Windows (64-bit only)
	  	#define X_SYSTEM_WIN64
	#else
		//define something for Windows (32-bit only)
		#define X_SYSTEM_WIN32
	#endif
#elif defined(__APPLE__)
	#define X_SYSTEM_APPLE
	#include <TargetConditionals.h>
	#if defined(TARGET_IPHONE_SIMULATOR)
		// iOS Simulator
		#define X_SYSTEM_IPHONE_SIMULATOR
	#elif defined(TARGET_OS_IPHONE)
		// iOS device
		#define X_SYSTEM_IPHONE
	#elif defined(TARGET_OS_MAC)
		// Other kinds of Mac OS
		#define X_SYSTEM_MACOS
	#else
	#	error "Unknown Apple platform"
	#endif
#elif defined(__ANDROID_API__)
	#define X_SYSTEM_LINUX
	#define X_SYSTEM_ANDROID
#elif defined(__linux__)
	// linux
	#define X_SYSTEM_LINUX
	#ifdef __FreeBSD__
		#define X_SYSTEM_FREEBSD
	#endif
#elif defined(__unix__) // all unices not caught above
	// Unix
	#error "unsupported unix"
#elif defined(_POSIX_VERSION)
	// POSIX
	#error "unsupported posix"
#else
	#error "Unknown system type"
#endif

#if defined(X_SYSTEM_WINDOWS) || defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_MACOS)
#ifndef X_SYSTEM_ANDROID
	#define X_SYSTEM_DESKTOP
#endif
#endif

#if defined(X_SYSTEM_IPHONE_SIMULATOR) || defined(X_SYSTEM_IPHONE)
	#define X_SYSTEM_IOS
#endif

#if defined(X_SYSTEM_MACOS) || defined(X_SYSTEM_IOS)
	#define X_SYSTEM_DARWIN
#endif

#if defined(_MSC_VER)

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#define X_LIKELY(x)                    (x)
	#define X_UNLIKELY(x)                  (x)
	#define X_PRINTF_LIKE(a, b)

	#define X_INLINE                       __forceinline
	#define X_STATIC_INLINE                static __forceinline

	#define X_PRIVATE                      extern
	#define X_PRIVATE_MEMBER
	#define X_PRIVATE_STATIC_MEMBER        static
	#define X_PRIVATE_CONSTEXPR            constexpr
	#define X_PRIVATE_STATIC_CONSTEXPR     static constexpr
	#define X_PRIVATE_INLINE               __forceinline

	#define X_EXPORT                       __declspec(dllexport) extern
	#define X_EXPORT_MEMBER                __declspec(dllexport)
	#define X_EXPORT_STATIC_MEMBER         __declspec(dllexport) static
	#define X_IMPORT                       __declspec(dllimport) extern
	#define X_IMPORT_MEMBER                __declspec(dllimport)
	#define X_IMPORT_STATIC_MEMBER         __declspec(dllimport) static

#elif defined(__clang__) || defined(__GNUC__)

	#define X_LIKELY(x)                    __builtin_expect(!!(x), 1)
	#define X_UNLIKELY(x)                  __builtin_expect(!!(x), 0)
	#define X_PRINTF_LIKE(f, a)            __attribute__((format(printf, f, a)))

	#define X_INLINE                       __attribute__((always_inline)) inline
	#define X_STATIC_INLINE                __attribute__((always_inline)) static inline

	#define X_PRIVATE                      __attribute__((__visibility__("hidden"))) extern
	#define X_PRIVATE_MEMBER               __attribute__((__visibility__("hidden")))
	#define X_PRIVATE_STATIC_MEMBER        __attribute__((__visibility__("hidden"))) static
	#define X_PRIVATE_CONSTEXPR            __attribute__((__visibility__("hidden"))) constexpr
	#define X_PRIVATE_STATIC_CONSTEXPR     __attribute__((__visibility__("hidden"))) static constexpr
	#define X_PRIVATE_INLINE               __attribute__((always_inline)) __attribute__((__visibility__("hidden"))) inline

	#define X_EXPORT                       __attribute__((__visibility__("default"))) extern
	#define X_EXPORT_MEMBER                __attribute__((__visibility__("default")))
	#define X_EXPORT_STATIC_MEMBER         __attribute__((__visibility__("default"))) static
	#define X_IMPORT                       extern
	#define X_IMPORT_MEMBER
	#define X_IMPORT_STATIC_MEMBER         static

#else
	#error "Unsupported compiler"
#endif

#define X_EXTERN             extern
#define X_MEMBER
#define X_STATIC_MEMBER      static
#if defined(X_OPTION_STATIC)
	#if defined(X_OPTION_EXPORT_API)
		#error X_OPTION_STATIC is used with X_OPTION_EXPORT_API
	#endif
	#define X_API                      X_EXTERN
	#define X_API_MEMBER               X_MEMBER
	#define X_API_STATIC_MEMBER        X_STATIC_MEMBER
#else
	#if defined(X_OPTION_EXPORT_API)
		#define X_API                  X_EXPORT
		#define X_API_MEMBER           X_EXPORT_MEMBER
		#define X_API_STATIC_MEMBER    X_EXPORT_STATIC_MEMBER
	#else
		#define X_API                  X_IMPORT
		#define X_API_MEMBER           X_IMPORT_MEMBER
		#define X_API_STATIC_MEMBER    X_IMPORT_STATIC_MEMBER
	#endif
#endif

#ifndef __cplusplus
#define X_CNAME
#define X_CNAME_BEGIN
#define X_CNAME_END
#else
#define X_CNAME extern "C"
#define X_CNAME_BEGIN extern "C" {
#define X_CNAME_END }
#endif

#define X_MAKE_STRING(s) #s
#define X_EXPAND_STRING(s) X_MAKE_STRING(s)
#define X_EXPECT(x) do { bool test=(bool)(x); if(!test) { throw #x; } } while(0)

#if defined(_MSC_VER)
#	define XelAlignedAlloc                      _aligned_malloc
#	define XelAlignedFree                       _aligned_free
#   define XelLocalTime(Timestamp, ST)          localtime_s((ST), (Timestamp));
#else
#	define XelAlignedAlloc(size, alignment)     aligned_alloc(alignment, size)
#	define XelAlignedFree                       free
#   define XelLocalTime(Timestamp, ST)          localtime_r((Timestamp), (ST));
#endif

#if defined(X_SYSTEM_ANDROID)
#	define _X_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName) \
		Java_ ## PackageName ## _ ## ClassFuncName
#	define X_ANDROID_API__FUNC_NAME(PackageName, ClassFuncName) \
		_X_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName)

#if defined(ANDROID_PACKAGE_CNAME)
#	define _X_ANDROID_API(ClassFuncName, ReturnType) \
		X_CNAME JNIEXPORT ReturnType JNICALL X_ANDROID_API__FUNC_NAME(ANDROID_PACKAGE_CNAME, ClassFuncName)
#	define X_ANDROID_API(ClassName, FuncName, ReturnType) _X_ANDROID_API(ClassName ## _ ## FuncName, ReturnType)
#	endif
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define __X_STRINGIFY__(x) #x
#define X_STRINGIFY(x) __X_STRINGIFY__(x)

X_CNAME_BEGIN

#define X_AddressOf(T) (&(T))
X_STATIC_INLINE void * X_Cast(void * MemberPtr, size_t MemberOffset)
{
	if (!MemberPtr) {
		return NULL;
	}
    return (void*)((unsigned char*)MemberPtr - MemberOffset);
}
#define X_Entry(_MemberPtr, Type, Member) ((Type*)(X_Cast((_MemberPtr), offsetof(Type, Member))))

X_CNAME_END
