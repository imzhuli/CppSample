#pragma once
#include "../Common.hpp"
#include <array>
#include <utility>

ZEC_NS
{

	template<typename T, size_t N>
	class xArray final
	{
	private:
		static_assert(!std::is_reference_v<T>);
	private:
		T _Array[N];
	public:
		template<typename CT, typename... CArgs>
		ZEC_INLINE constexpr xArray(const std::in_place_type_t<CT> &, CArgs&& ... args)
			: _Array{std::forward<CArgs>(args)...}
		{}

		ZEC_INLINE constexpr T * Data() { return _Array; }
		ZEC_INLINE constexpr const T * Data() const { return _Array; }
		ZEC_INLINE constexpr size_t Size() const { return N; }

		ZEC_INLINE constexpr operator T * () { return _Array; }
		ZEC_INLINE constexpr operator const T * () const { return _Array; }
		ZEC_INLINE constexpr T & operator[] (ptrdiff_t Offset) { return _Array[Offset]; }
		ZEC_INLINE constexpr const T & operator[] (ptrdiff_t Offset) const { return _Array[Offset]; }

		ZEC_INLINE constexpr T * begin() { return _Array; }
		ZEC_INLINE constexpr const T * begin() const { return _Array; }
		ZEC_INLINE constexpr T * end() { return _Array + N; }
		ZEC_INLINE constexpr const T * end() const { return _Array + N; }
	} ;

	template<typename T>
	class xArray<T, 0> final
	{
	private:
		static_assert(!std::is_reference_v<T>);
	private:
		constexpr static const size_t N = 0;
		alignas(T) ubyte _Array[sizeof(T)];
	public:
		ZEC_INLINE constexpr T * Data() { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr const T * Data() const { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr size_t Size() const { return N; }

		ZEC_INLINE constexpr operator T * () { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr operator const T * () const { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr T & operator[] (ptrdiff_t Offset) { return reinterpret_cast<T*>(_Array)[Offset]; }
		ZEC_INLINE constexpr const T & operator[] (ptrdiff_t Offset) const { return reinterpret_cast<T*>(_Array)[Offset]; }

		ZEC_INLINE constexpr T * begin() { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr const T * begin() const { return reinterpret_cast<T*>(_Array); }
		ZEC_INLINE constexpr T * end() { return reinterpret_cast<T*>(_Array) + N; }
		ZEC_INLINE constexpr const T * end() const { return reinterpret_cast<T*>(_Array) + N; }
	};

	template<typename CT, typename... CArgs>
	xArray(const std::in_place_type_t<CT> &, CArgs&& ... args) -> xArray<CT, sizeof...(CArgs)>;

}
