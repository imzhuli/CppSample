#pragma once
#include <zec/Common.hpp>
#include <cstring>

X_NS
{

	class xUuid
	{
	public:
		using xRawType = ubyte[16];

		X_INLINE const xRawType&  GetData() const { return _Data; }
		X_INLINE constexpr size_t GetSize() const { return sizeof(xRawType); }

		X_INLINE xUuid() : _Data{} {}
		X_INLINE xUuid(const xNoInit &) {}
		X_INLINE xUuid(const xGeneratorInit &) { Generate(); }
		X_INLINE xUuid(const xRawType & RawData) { memcpy(&_Data, &RawData, sizeof(xRawType)); }

		X_INLINE bool operator != (const xUuid & Other) const { return memcmp(_Data, Other._Data, sizeof(xRawType)); }
		X_INLINE bool operator == (const xUuid & Other) const { return !memcmp(_Data, Other._Data, sizeof(xRawType)); }
		X_INLINE bool operator < (const xUuid & Other) const { return memcmp(_Data, Other._Data, 16) < 0; }
		X_INLINE bool operator > (const xUuid & Other) const { return memcmp(_Data, Other._Data, 16) > 0; }
		X_INLINE bool operator <= (const xUuid & Other) const { return memcmp(_Data, Other._Data, 16) <= 0; }
		X_INLINE bool operator >= (const xUuid & Other) const { return memcmp(_Data, Other._Data, 16) >= 0; }

		X_API_MEMBER bool Generate();

	private:
		xRawType _Data;
	};

}
