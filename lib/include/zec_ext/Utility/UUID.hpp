#pragma once
#include <zec/Common.hpp>

ZEC_NS
{

	class xUUID
	{
	public:
		using DataType = ubyte[16];

		ZEC_INLINE const DataType&  GetData() const { return _Data; }
		ZEC_INLINE constexpr size_t Size() const { return 16; }

		xUUID() : _Data{} {};
		ZEC_API_MEMBER void Generate();

	private:
		DataType _Data;
	};

}
