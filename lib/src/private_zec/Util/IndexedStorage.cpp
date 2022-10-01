#include <zec/Util/IndexedStorage.hpp>
#include <zec/Util/Chrono.hpp>

ZEC_NS
{
	static_assert(sizeof(xIndexId) == sizeof(uint64_t));

	uint32_t xIndexId::TimeSeed()
	{
		return static_cast<uint32_t>(GetTimestampUS());
	}

}
