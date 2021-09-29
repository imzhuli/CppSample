#include <zec/Util/IndexedStorage.hpp>
#include <zec/Util/Chrono.hpp>

ZEC_NS
{
	uint_fast32_t xIndexId::TimeSeed()
	{
		return static_cast<uint_fast32_t>(GetMicroTimestamp());
	}

}
