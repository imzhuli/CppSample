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
	#define ZEC_SYSTEM_GENERIC
#endif

#if defined(_MSC_VER)
	#define NOMINMAX
	#define ZEC_INLINE                       __forceinline
	#define ZEC_STATIC_INLINE                static __forceinline
	#if defined(ZEC_OPTION_STATIC)
		#if defined(ZEC_OPTION_EXPORT_API)
			#error ZEC_OPTION_STATIC is used with ZEC_OPTION_EXPORT_API
		#endif
		#define ZEC_API                      extern
		#define ZEC_API_MEMBER
		#define ZEC_API_STATIC_MEMBER        static
	#else
		#if defined(ZEC_OPTION_EXPORT_API)
			#define ZEC_API                  __declspec(dllexport) extern
			#define ZEC_API_MEMBER           __declspec(dllexport)
			#define ZEC_API_STATIC_MEMBER    __declspec(dllexport) static
			#define ZEC_API_FRIEND           friend __declspec(dllexport)
		#else
			#define ZEC_API                  __declspec(dllimport) extern
			#define ZEC_API_MEMBER           __declspec(dllimport)
			#define ZEC_API_STATIC_MEMBER    __declspec(dllimport) static
			#define ZEC_API_FRIEND           friend __declspec(dllimport)
		#endif
	#endif
	#define ZEC_PRIVATE                      extern
	#define ZEC_PRIVATE_MEMBER
	#define ZEC_PRIVATE_STATIC_MEMBER        static
	#define ZEC_PRIVATE_CONSTEXPR            constexpr
	#define ZEC_PRIVATE_STATIC_CONSTEXPR     static constexpr
	#define ZEC_PRIVATE_INLINE               __forceinline
	#define ZEC_EXPORT                       __declspec(dllexport) extern
	#define ZEC_IMPORT                       __declspec(dllimport) extern
#elif defined(__clang__) || defined(__GNUC__)
	#define ZEC_INLINE                       __attribute__((always_inline)) inline
	#define ZEC_STATIC_INLINE                __attribute__((always_inline)) static inline
	#if defined(ZEC_OPTION_STATIC)
		#if defined(ZEC_OPTION_EXPORT_API)
			#error ZEC_OPTION_STATIC is used with ZEC_OPTION_EXPORT_API
		#endif
		#define ZEC_API                      extern
		#define ZEC_API_MEMBER
		#define ZEC_API_STATIC_MEMBER        static
	#else
		#if defined(ZEC_OPTION_EXPORT_API)
			#define ZEC_API                  __attribute__((__visibility__("default"))) extern
			#define ZEC_API_MEMBER           __attribute__((__visibility__("default")))
			#define ZEC_API_STATIC_MEMBER    __attribute__((__visibility__("default"))) static
			#define ZEC_API_FRIEND           friend __attribute__((__visibility__("default")))
		#else
			#define ZEC_API                  extern
			#define ZEC_API_MEMBER
			#define ZEC_API_STATIC_MEMBER    static
			#define ZEC_API_FRIEND           friend
		#endif
	#endif
	#define ZEC_PRIVATE                      __attribute__((__visibility__("hidden"))) extern
	#define ZEC_PRIVATE_MEMBER               __attribute__((__visibility__("hidden")))
	#define ZEC_PRIVATE_STATIC_MEMBER        __attribute__((__visibility__("hidden"))) static
	#define ZEC_PRIVATE_CONSTEXPR            __attribute__((__visibility__("hidden"))) constexpr
	#define ZEC_PRIVATE_STATIC_CONSTEXPR     __attribute__((__visibility__("hidden"))) static constexpr
	#define ZEC_PRIVATE_INLINE               __attribute__((always_inline)) __attribute__((__visibility__("hidden"))) inline
	#define ZEC_EXPORT                       __attribute__((__visibility__("default"))) extern
	#define ZEC_IMPORT                       extern
#else
	#error "Unsupported compiler"
#endif


#include <new>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#define ZEC_NS namespace zec

#define ZEC_MAKE_STRING(s) #s
#define ZEC_EXPAND_STRING(s) ZEC_MAKE_STRING(s)
#define ZEC_EXPECT(x) do { bool test=(bool)(x); if(!test) { throw #x; } } while(0)

#if defined(_MSC_VER)
#	define ZecAlignedAlloc                      _aligned_malloc
#	define ZecAlignedFree                       _aligned_free
#else
#	define ZecAlignedAlloc(size, alignment)     aligned_alloc(alignment, size)
#	define ZecAlignedFree                       free
#endif

#if defined(ZEC_SYSTEM_ANDROID)
#	define _ZEC_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName) \
		Java_ ## PackageName ## _ ## ClassFuncName
#	define ZEC_ANDROID_API__FUNC_NAME(PackageName, ClassFuncName) \
		_ZEC_ANDROID_API__FUNC_NAME_JOIN(PackageName, ClassFuncName)

#if defined(ANDROID_PACKAGE_CNAME)
#	define _ZEC_ANDROID_API(ClassFuncName, ReturnType) \
		extern "C" JNIEXPORT ReturnType JNICALL ZEC_ANDROID_API__FUNC_NAME(ANDROID_PACKAGE_CNAME, ClassFuncName)
#	define ZEC_ANDROID_API(ClassName, FuncName, ReturnType) _ZEC_ANDROID_API(ClassName ## _ ## FuncName, ReturnType)
#	endif
#endif

ZEC_NS
{

	inline namespace common
	{

		inline namespace numeric
		{
			using byte  = ::std::byte;
			using ubyte = unsigned char;

			using size8_t      = ::std::uint8_t;
			using size16_t     = ::std::uint16_t;
			using size32_t     = ::std::uint32_t;
			using size64_t     = ::std::uint64_t;

			using ssize8_t     = ::std::int8_t;
			using ssize16_t    = ::std::int16_t;
			using ssize32_t    = ::std::int32_t;
			using ssize64_t    = ::std::int64_t;

			using offset8_t    = ::std::int8_t;
			using offset16_t   = ::std::int16_t;
			using offset32_t   = ::std::int32_t;
			using offset64_t   = ::std::int64_t;
			using offset_t     = ::std::ptrdiff_t;

			using size_t       = ::std::size_t;
			using ssize_t      = typename ::std::make_signed<size_t>::type;

		} // namespace numeric

		union xVariable
		{
			void *                        Ptr;
			const void *                  CstPtr;
			const char *                  Str;

			ptrdiff_t                     Offset;
			size_t                        Size;

			int                           I;
			unsigned int                  U;
			int8_t                        I8;
			int16_t                       I16;
			int32_t                       I32;
			int64_t                       I64;
			uint8_t                       U8;
			uint16_t                      U16;
			uint32_t                      U32;
			uint64_t                      U64;

			struct { int32_t  x, y; }     VI32;
			struct { uint32_t x, y; }     VU32;
		};

		template<typename T>
		constexpr std::in_place_type_t<T> xType {};

		constexpr struct xNoInit {} NoInit {};
		constexpr struct xZeroInit {} ZeroInit {};
		constexpr struct xDefaultInit {} DefaultInit {};
		constexpr struct xGeneratorInit {} GeneratorInit {};

		struct xSizeInit final { size_t value; };
		struct xCapacityInit final { size_t value; };
		struct xPass final { ZEC_INLINE void operator()() const {} };
		struct xAbstract { protected: xAbstract() = default; virtual ~xAbstract() = default; xAbstract(xAbstract &&) = delete; };
		struct xNonCopyable { protected: xNonCopyable() = default; ~xNonCopyable() = default; xNonCopyable(xNonCopyable &&) = delete; };

		template<typename T1, typename T2>
		using xDiffType = decltype(std::declval<T1>() - std::declval<T2>());
		template<typename T1, typename T2> ZEC_INLINE constexpr auto Diff(const T1& Value, const T2& ComparedToValue) { return Value - ComparedToValue; }
		template<typename T1, typename T2> ZEC_INLINE constexpr auto SignedDiff(const T1& Value, const T2& ComparedToValue) { return static_cast<std::make_signed_t<xDiffType<T1, T2>>>(Value - ComparedToValue); }
		template<typename T1, typename T2> ZEC_INLINE constexpr auto UnsignedDiff(const T1& Value, const T2& ComparedToValue) { return static_cast<std::make_unsigned_t<xDiffType<T1, T2>>>(Value - ComparedToValue); }

		ZEC_STATIC_INLINE void Pass() {};
		ZEC_STATIC_INLINE void Error() { throw nullptr; }
		ZEC_STATIC_INLINE void Error(const char * message) { throw message; }
		ZEC_STATIC_INLINE void Todo() { Error("Not implmented"); }
		ZEC_STATIC_INLINE void Todo(const char * info) { Error(info); }
		ZEC_STATIC_INLINE void Pure() { Error("placeholder of pure function called, which is not expected"); }
		ZEC_STATIC_INLINE constexpr const char * YN(bool y) { return y ? "yes" : "no"; }
		ZEC_STATIC_INLINE constexpr const char * TF(bool t) { return t ? "true" : "false"; }

		template<typename T>
		ZEC_STATIC_INLINE bool IsDefaultValue(const T& Target) { return Target == T{}; }

		template<typename T, typename TValue>
		ZEC_STATIC_INLINE void
		Assign(T& ExpiringTarget,  TValue && value) { ExpiringTarget = std::forward<TValue>(value); }

		template<typename T>
		ZEC_STATIC_INLINE void
		Reset(T& ExpiringTarget) { ExpiringTarget = T{}; }

		template<typename T, typename TValue>
		ZEC_STATIC_INLINE void
		Reset(T& ExpiringTarget,  TValue && value) { ExpiringTarget = std::forward<TValue>(value); }

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE T
		Steal(T& ExpiringTarget) {
			T ret = ExpiringTarget;
			ExpiringTarget = T{};
			return ret;
		}

		template<typename T, typename TValue>
		[[nodiscard]] ZEC_STATIC_INLINE T
		Steal(T& ExpiringTarget,  TValue && value) {
			T ret = ExpiringTarget;
			ExpiringTarget = std::forward<TValue>(value);
			return ret;
		}

		template<typename T, size_t L>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr size_t
		Length(const T(&)[L]) { return L; }

		template<typename T, size_t L>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr size_t
		SafeLength(const T(&)[L]) { return L ? L - 1 : L; }

		template<typename... Args>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr size_t
		Count(const Args& ... args) { return sizeof...(args); }

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr std::conditional_t<std::is_const_v<T>, const void *, void *>
		AddressOf(T & obj) { return &reinterpret_cast<std::conditional_t<std::is_const_v<T>, const unsigned char, unsigned char>&>(obj); }

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr bool
		IsPow2(const T x) { static_assert(std::is_integral_v<T>); return x > 0 && !(x & (x-1)); }

		template<typename tOffsetType, typename tAlignmentType>
		[[nodiscard]] ZEC_STATIC_INLINE constexpr tOffsetType
		Align(tOffsetType Origin, tAlignmentType Alignment) { assert(IsPow2(Alignment) && Alignment != 1); const auto Offset = Alignment - 1; return (Origin + Offset) & ~Offset; }

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE std::remove_reference_t<T> &
		X2Ref(T && ref) { return ref; }

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE std::remove_reference_t<T> *
		X2Ptr(T && ref) { return &ref; }

		// Util classes:

		template<typename T>
		class xRef final {
		public:
			[[nodiscard]] constexpr explicit xRef(T & Ref) noexcept : _Ref(&Ref) {}
			[[nodiscard]] constexpr xRef(const xRef & RRef) noexcept = default;
			ZEC_INLINE constexpr T& Get() const noexcept { return *_Ref; }
		private:
			T * _Ref;
		};

		template<typename T, typename tExit>
		class xScopedPtr final
		: xNonCopyable {
			T * _Ptr;
			const tExit _ExitCallback;
		public:
			[[nodiscard]] ZEC_INLINE xScopedPtr(T * Ptr, const tExit& Exit) : _Ptr(Ptr), _ExitCallback(Exit) {}
			ZEC_INLINE ~xScopedPtr() { _ExitCallback(_Ptr); }
		public:
			[[nodiscard]] operator T * ()  const { return _Ptr; }
			[[nodiscard]] T * operator->() const { return _Ptr; }
			[[nodiscard]] T & operator *() const { return *_Ptr; }
		};
		template<typename tValue, typename tExit>
		xScopedPtr(tValue * && Ptr, const tExit& Exit) -> xScopedPtr<tValue, std::decay_t<tExit>>;

		template<typename tFuncObj, typename ... Args>
		struct xInstantRun final : xNonCopyable	{
			ZEC_INLINE xInstantRun(tFuncObj && Func, Args&& ... args) { std::forward<tFuncObj>(Func)(std::forward<Args>(args)...); }
		};

		template<typename tEntry, typename tExit>
		class xScopeGuard final : xNonCopyable {
			/** NOTE: It's important typeof(_ExitCallback) is not reference,
			 *  so that it be compatible with:
			 *     function,
			 *     lambda (w/o worrying about capturing-lambda function's lifetime),
			 *     and const func-object (which is often with inline trivial ctor(default/copy/move) and dtor).
			 *  if caller is quite aware of the lifetime of a func-object and if:
			 *       the fuct-object is non-copyable, or
			 *       avoiding ctor/copy ctor/dtor really matters
			 *     use xRef(some_non_const_object) bellow as a const-wrapper-object
			 * */
			const tExit _ExitCallback;
		public:
			[[nodiscard]] ZEC_INLINE xScopeGuard(const tEntry& Entry, const tExit& Exit) : _ExitCallback(Exit) { Entry(); }
			[[nodiscard]] ZEC_INLINE xScopeGuard(const tExit& Exit) : _ExitCallback(Exit) {}
			ZEC_INLINE ~xScopeGuard() { _ExitCallback(); }
		};
		template<typename tEntry, typename tExit>
		xScopeGuard(const tEntry& Entry, const tExit& Exit) -> xScopeGuard<std::decay_t<tEntry>, std::decay_t<tExit>>;
		template<typename tExit>
		xScopeGuard(const tExit& Exit) -> xScopeGuard<xPass, std::decay_t<tExit>>;

		template<typename T>
		class xResourceGuard final : xNonCopyable {
		public:
			template<typename ... tArgs>
			ZEC_INLINE constexpr xResourceGuard(T & Resource, tArgs&& ... Args) : _Resource(Resource), _Inited(Resource.Init(std::forward<tArgs>(Args)...)) {}
			ZEC_INLINE ~xResourceGuard() { if (_Inited) {_Resource.Clean();} }
			ZEC_INLINE operator bool () const { return _Inited; }
		private:
			T & _Resource;
			const bool _Inited;
		};

		template<typename T, typename ... tArgs>
		xResourceGuard(T & Resource, tArgs&& ... Args) -> xResourceGuard<T>;

		template<typename T>
		class xOptional final {
			static_assert(!std::is_reference_v<T> && !std::is_const_v<T>);
			using Type = std::remove_cv_t<std::remove_reference_t<T>>;

			template<typename RefedT>
			struct xRefCaster {
				static_assert(!std::is_reference_v<RefedT>);
				using Type = RefedT;
				static RefedT& Get(RefedT & R) { return R; }
				static const RefedT& Get(const RefedT & R) { return R; }
			};
			template<typename RefedT>
			struct xRefCaster<xRef<RefedT>> {
				static_assert(!std::is_reference_v<RefedT>);
				using Type = RefedT;
				static RefedT& Get(const xRef<RefedT> & RR) { return RR.Get(); }
			};
			using xCaster = xRefCaster<T>;
			using xValueType = typename xCaster::Type;

		public:
			ZEC_INLINE xOptional() = default;
			ZEC_INLINE ~xOptional() { if(_Valid) { Destroy(); } }
			ZEC_INLINE xOptional(const xOptional & Other) {
				if (Other._Valid) {
					new ((void*)_Holder) Type(Other.GetReference());
					_Valid = true;
				}
			}
			ZEC_INLINE xOptional(xOptional && Other) {
				if (Other._Valid) {
					new ((void*)_Holder) Type(std::move(Other.GetReference()));
					_Valid = true;
				}
			}
			template<typename U>
			ZEC_INLINE xOptional(U&& Value) {
				new ((void*)_Holder) Type(std::forward<U>(Value));
				_Valid = true;
			}

			ZEC_INLINE xOptional & operator = (const xOptional &Other) {
				if (_Valid) {
					if (Other._Valid) {
						GetReference() = Other.GetReference();
					} else {
						Destroy();
						_Valid = false;
					}
				} else {
					if (Other._Valid) {
						new ((void*)_Holder) Type(Other.GetReference());
						_Valid = true;
					}
				}
				return *this;
			}
			ZEC_INLINE xOptional & operator = (xOptional && Other) {
				if (_Valid) {
					if (Other._Valid) {
						GetReference() = std::move(Other.GetReference());
					} else {
						Destroy();
						_Valid = false;
					}
				} else {
					if (Other._Valid) {
						new ((void*)_Holder) Type(std::move(Other.GetReference()));
						_Valid = true;
					}
				}
				return *this;
			}
			template<typename U>
			ZEC_INLINE xOptional & operator = (U&& Value) {
				if (!_Valid) {
					new ((void*)_Holder) Type(std::forward<U>(Value));
					_Valid = true;
				} else {
					GetReference() = std::forward<U>(Value);
				}
				return *this;
			}

			ZEC_INLINE void Reset() { Steal(_Valid) ? Destroy() : Pass(); }

			ZEC_INLINE bool operator()() const { return _Valid; }

			ZEC_INLINE auto & operator *() { assert(_Valid); return GetValueReference(); }
			ZEC_INLINE auto & operator *() const { assert(_Valid); return GetValueReference(); }

			ZEC_INLINE auto * operator->() { return _Valid ? &GetValueReference() : nullptr; }
			ZEC_INLINE auto * operator->() const { return _Valid ? &GetValueReference() : nullptr; }

			ZEC_INLINE const xValueType & Or(const xValueType & DefaultValue) const { return _Valid ? GetValueReference() : DefaultValue; }

		private:
			ZEC_INLINE Type & GetReference() { return reinterpret_cast<Type&>(_Holder); }
			ZEC_INLINE const Type & GetReference() const { return reinterpret_cast<const Type&>(_Holder); }
			ZEC_INLINE auto & GetValueReference() { return xCaster::Get(GetReference()); }
			ZEC_INLINE auto & GetValueReference() const { return xCaster::Get(GetReference()); }
			ZEC_INLINE void Destroy() { GetReference().~Type(); }

		private:
			bool _Valid {};
			alignas(Type) ubyte _Holder[sizeof(Type)];
		};

		template<typename U>
		xOptional(const U& Value) -> xOptional<U>;

		template<typename U>
		xOptional(U&& Value) ->  xOptional<U>;

	}

}
