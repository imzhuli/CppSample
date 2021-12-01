#include <zec/Util/IndexedStorage.hpp>
#include <zec/Util/Chrono.hpp>

ZEC_NS
{
	static_assert(sizeof(xIndexId) == sizeof(uint64_t));

	uint_fast32_t xIndexId::TimeSeed()
	{
		return static_cast<uint_fast32_t>(GetMicroTimestamp());
	}

}
