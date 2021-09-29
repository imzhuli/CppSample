#pragma once
#include "./Common.hpp"
#include <array>
#include <utility>

ZEC_NS
{

	template<typename T, size_t N>
	class xCArray final
		: protected std::array<T, N>
	{
	private:
		using xElementType = std::decay_t<T>;
		using xUnderlyingType = std::array<xElementType, N>;

	public:
		template<typename CT, typename... CArgs>
		ZEC_INLINE constexpr xCArray(const std::in_place_type_t<CT> &, CArgs&& ... args)
			: xUnderlyingType{std::forward<CArgs>(args)...}
		{}

		using xUnderlyingType::operator[];
		using xUnderlyingType::begin;
		using xUnderlyingType::end;

		ZEC_INLINE constexpr auto Size() const { return xUnderlyingType::size(); }
		ZEC_INLINE constexpr T * Data(T * EmptyValue = nullptr) { return N ? xUnderlyingType::data() : EmptyValue; }
		ZEC_INLINE constexpr const T * Data(const T * EmptyValue = nullptr) const { return N ? xUnderlyingType::data() : EmptyValue; }

		ZEC_INLINE constexpr operator T * () { return Data(); }
		ZEC_INLINE constexpr operator const T * () const { return Data(); }
	} ;

	template<typename CT, typename... CArgs>
	xCArray(const std::in_place_type_t<CT> &, CArgs&& ... args) -> xCArray<std::decay_t<CT>, sizeof...(CArgs)>;

}
