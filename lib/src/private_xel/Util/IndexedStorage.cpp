#include <xel/Util/IndexedStorage.hpp>
#include <xel/Util/Chrono.hpp>

X_NS
{
	static_assert(sizeof(xIndexId) == sizeof(uint64_t));

	uint32_t xIndexId::TimeSeed()
	{
		return static_cast<uint32_t>(GetTimestampUS());
	}

}
