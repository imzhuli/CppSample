#include <zec/Byte.hpp>

ZEC_NS
{

    namespace __detail__::__raw__
    {

        static_assert(sizeof(UF) == sizeof(uint32_t));
        static_assert(sizeof(UD) == sizeof(uint64_t));

    }

}
