#pragma once

#include "./C/ZEC_Common.h"

#include <new>
#include <utility>
#include <type_traits>
#include <atomic>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#define ZEC_NS namespace zec

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
			ubyte                         B[8];

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


		namespace __detail__ {
			template< class T >
			struct xRemoveCVR {
				typedef std::remove_cv_t<std::remove_reference_t<T>> Type;
			};
		}
		template< class T >
		using xNoCVR = typename __detail__::xRemoveCVR<T>::Type;

		template<typename T>
		constexpr std::in_place_type_t<T> xType {};

		constexpr struct xNoInit final {} NoInit {};
		constexpr struct xZeroInit final {} ZeroInit {};
		constexpr struct xDefaultInit final {} DefaultInit {};
		constexpr struct xGeneratorInit final {} GeneratorInit {};
		constexpr struct xSizeInit final { size_t value; } ZeroSizeInit {};
		constexpr struct xCapacityInit final { size_t value; } ZeroCapacityInit{};
		
		struct xPass final { ZEC_INLINE void operator()() const {} };
		struct xAbstract { protected: xAbstract() = default; virtual ~xAbstract() = default; xAbstract(xAbstract &&) = delete; };
		struct xNonCopyable { protected: xNonCopyable() = default; ~xNonCopyable() = default; xNonCopyable(xNonCopyable &&) = delete; };
    	struct xNoCatch final { private: xNoCatch() = default; ~xNoCatch() = default; };

		template<typename T1, typename T2>
		using xDiff = decltype(std::declval<T1>() - std::declval<T2>());
		template<typename T1, typename T2> ZEC_INLINE constexpr auto Diff(const T1& Value, const T2& ComparedToValue) { return Value - ComparedToValue; }
		template<typename T1, typename T2> ZEC_INLINE constexpr auto SignedDiff(const T1& Value, const T2& ComparedToValue) { return static_cast<std::make_signed_t<xDiff<T1, T2>>>(Value - ComparedToValue); }
		template<typename T1, typename T2> ZEC_INLINE constexpr auto UnsignedDiff(const T1& Value, const T2& ComparedToValue) { return static_cast<std::make_unsigned_t<xDiff<T1, T2>>>(Value - ComparedToValue); }

		ZEC_STATIC_INLINE void Pass() {};
		ZEC_STATIC_INLINE void Error() { throw nullptr; }
		ZEC_STATIC_INLINE void Error(const char * message) { throw message; }
		ZEC_STATIC_INLINE void Fatal() { std::abort(); }
		ZEC_STATIC_INLINE void Fatal(const char *) { std::abort(); }
		ZEC_STATIC_INLINE void Todo() { Fatal(); }
		ZEC_STATIC_INLINE void Todo(const char * info) { Fatal(); }
		ZEC_STATIC_INLINE void Pure() { Fatal("placeholder of pure function called, which is not expected"); }
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
		ZEC_STATIC_INLINE void
		Renew(T& ExpiringTarget) { 
			ExpiringTarget.~T();
			new ((void*)&ExpiringTarget) T;
		}

		template<typename T, typename...tArgs>
		ZEC_STATIC_INLINE void
		RenewWith(T& ExpiringTarget,  tArgs && ... Args) { 
			ExpiringTarget.~T();
			new ((void*)&ExpiringTarget) T {std::forward<tArgs>(Args)...};
		}

		template<typename T>
		[[nodiscard]] ZEC_STATIC_INLINE T
		Steal(T& ExpiringTarget) {
			T ret = std::move(ExpiringTarget);
			ExpiringTarget = T{};
			return ret;
		}

		template<typename T, typename TValue>
		[[nodiscard]] ZEC_STATIC_INLINE T
		Steal(T& ExpiringTarget,  TValue && value) {
			T ret = std::move(ExpiringTarget);
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
			 *     and func-object (which is often with inline trivial ctor(default/copy/move) and dtor).
			 *  if caller is quite aware of the lifetime of a func-object and if:
			 *       the fuct-object is non-copyable, or
			 *       avoiding ctor/copy ctor/dtor really matters
			 *     use xRef(some_non_const_object) above as a const-wrapper-object
			 * */
			tExit _ExitCallback;
		public:
			[[nodiscard]] ZEC_INLINE xScopeGuard(const tEntry& Entry, const tExit& Exit) : _ExitCallback(Exit) { Entry(); }
			[[nodiscard]] ZEC_INLINE xScopeGuard(const tExit& Exit) : _ExitCallback(Exit) {}
			ZEC_INLINE ~xScopeGuard() { xRefCaster<tExit>::Get(_ExitCallback)(); }
		};
		template<typename tEntry, typename tExit>
		xScopeGuard(const tEntry& Entry, const tExit& Exit) -> xScopeGuard<std::decay_t<tEntry>, std::decay_t<tExit>>;
		template<typename tExit>
		xScopeGuard(const tExit& Exit) -> xScopeGuard<xPass, std::decay_t<tExit>>;

		namespace __detail__ {
			template<bool IsAtomic = false>
			class xReentryFlag final : xNonCopyable
			{
			private:
				using xCounter = std::conditional_t<IsAtomic, std::atomic_uint64_t, uint64_t>;
				class xGuard : xNonCopyable
				{
				public:
					xGuard(xCounter& CounterRef) : _CounterRef(CounterRef), _Entered(!CounterRef++) {};
					xGuard(xGuard && Other) : _CounterRef(Other._CounterRef), _Entered(Steal(Other._Entered)) { ++_CounterRef; };
					~xGuard() { --_CounterRef; }
					operator bool () const { return _Entered; }

				private:
					xCounter &   _CounterRef;
					bool         _Entered;
					friend class xReentryFlag;
				};
			public:
				xGuard Guard() { return xGuard(_Counter); }

			private:
				xCounter _Counter {};

			};
		}

		using xReentryFlag = __detail__::xReentryFlag<false>;
		using xAtomicReentryFlag = __detail__::xReentryFlag<true>;

		template<typename T>
		class xResourceGuard final : xNonCopyable {
		public:
			template<typename ... tArgs>
			ZEC_INLINE constexpr xResourceGuard(T & Resource, tArgs&& ... Args) : _Resource(Resource), _Inited(Resource.Init(std::forward<tArgs>(Args)...)) {}
			ZEC_INLINE constexpr xResourceGuard(T && Other) : _Resource(Other._Resource), _Inited(Steal(Other._Inited)) {}
			ZEC_INLINE ~xResourceGuard() { if (_Inited) {_Resource.Clean();} }
			ZEC_INLINE operator bool () const { return _Inited; }
		private:
			T & _Resource;
			const bool _Inited;
		};

		template<typename T, typename ... tArgs>
		xResourceGuard(T & Resource, tArgs&& ... Args) -> xResourceGuard<T>;

		/* change variable value, and reset it to its original value after scope of the guard */
		template<typename T>
		class xValueGuard final : xNonCopyable
		{
		public:
			template<typename tU>
			xValueGuard(T & Ref, tU && U) : _Ref(Ref), _OriginalValue(std::move(Ref)) {
				_Ref = std::forward<tU>(U);
			}
			~xValueGuard() {
				_Ref = std::move(_OriginalValue);
			}
		private:
			T & _Ref;
			T _OriginalValue;
		};

		template<size_t TargetSize, size_t Alignment = alignof(std::max_align_t)>
		class xDummy final
		: xNonCopyable
		{
		public:
			template<typename T>
			ZEC_INLINE void CreateAs() {
                static_assert(Alignment >= alignof(T));
				static_assert(sizeof(_PlaceHolder) >= sizeof(T));
				new ((void*)_PlaceHolder) T;
			}

			template<typename T, typename ... tArgs>
			ZEC_INLINE T& CreateValueAs(tArgs && ... Args) {
                static_assert(Alignment >= alignof(T));
				static_assert(sizeof(_PlaceHolder) >= sizeof(T));
				return *(new ((void*)_PlaceHolder) T(std::forward<tArgs>(Args)...));
			}

			template<typename T>
			ZEC_INLINE void DestroyAs() {
                static_assert(Alignment >= alignof(T));
				static_assert(sizeof(_PlaceHolder) >= sizeof(T));
				reinterpret_cast<T*>(_PlaceHolder)->~T();
			}

			template<typename T>
			ZEC_INLINE T & As() {
                static_assert(Alignment >= alignof(T));
				static_assert(sizeof(_PlaceHolder) >= sizeof(T));
				return reinterpret_cast<T&>(_PlaceHolder);
			}

			template<typename T>
			ZEC_INLINE const T & As() const {
                static_assert(Alignment >= alignof(T));
				static_assert(sizeof(_PlaceHolder) >= sizeof(T));
				return reinterpret_cast<const T&>(_PlaceHolder);
			}

			static constexpr const size_t Size = TargetSize;

		private:
			alignas(Alignment) ubyte _PlaceHolder[TargetSize];
		};


		template<typename T>
		class xHolder final
		: xNonCopyable
		{
		public:
			ZEC_INLINE xHolder() = default;
			ZEC_INLINE ~xHolder() = default;

			ZEC_INLINE void Create() {
				new ((void*)_PlaceHolder) T;
			}

			template<typename ... tArgs>
			ZEC_INLINE T* CreateValue(tArgs && ... Args) {
				auto ObjectPtr = new ((void*)_PlaceHolder) T(std::forward<tArgs>(Args)...);
				return ObjectPtr;
			}

			ZEC_INLINE void Destroy() {
				Get()->~T();
			}

			ZEC_INLINE T * operator->() { return Get(); }
			ZEC_INLINE const T * operator->() const { return Get(); }

			ZEC_INLINE T & operator*() { return *Get(); }
			ZEC_INLINE const T & operator*() const { return *Get(); }

			ZEC_INLINE T * Get() { return reinterpret_cast<T*>(_PlaceHolder); }
			ZEC_INLINE const T * Get() const { return reinterpret_cast<const T*>(_PlaceHolder); }

		private:
			alignas(T) ubyte _PlaceHolder [sizeof(T)];
		};

		template<typename T>
		class xOptional final {
			static_assert(!std::is_reference_v<T> && !std::is_const_v<T>);
			using Type = std::remove_cv_t<std::remove_reference_t<T>>;
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
