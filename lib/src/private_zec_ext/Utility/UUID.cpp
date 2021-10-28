#include <zec/Common.hpp>
#include <zec_ext/Utility/UUID.hpp>

#define BOOST_UUID_FORCE_AUTO_LINK
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators

ZEC_NS
{
	static auto UuidGenerator = boost::uuids::random_generator();

	void xUUID::Generate()
	{
		auto UUID = UuidGenerator();
		memcpy(&_Data, &UUID, 16);
	}

}
