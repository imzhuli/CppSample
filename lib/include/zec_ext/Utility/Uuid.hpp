#pragma once
#include <zec/Common.hpp>
#include <cstring>

ZEC_NS
{

	class xUuid
	{
	public:
		using xRawType = ubyte[16];

		ZEC_INLINE const xRawType&  GetData() const { return _Data; }
		ZEC_INLINE constexpr size_t GetSize() const { return sizeof(xRawType); }

		ZEC_INLINE xUuid() : _Data{} {}
		ZEC_INLINE xUuid(const xNoInit &) {}
		ZEC_INLINE xUuid(const xGeneratorInit &) { Generate(); }
		ZEC_INLINE xUuid(const xRawType & RawData) { memcpy(&_Data, &RawData, sizeof(xRawType)); }

		// ZEC_INLINE xUuid(const xUuid &) = default;
		ZEC_INLINE bool operator != (const xUuid & Other) const { return memcmp(_Data, Other._Data, sizeof(xRawType)); }
		ZEC_INLINE bool operator == (const xUuid & Other) const { return !memcmp(_Data, Other._Data, sizeof(xRawType)); }

		ZEC_API_MEMBER void Generate();

	private:
		xRawType _Data;
	};

}
