#pragma once
#include "../Common.hpp"

ZEC_NS
{

    template<typename tOperator>
    ZEC_INLINE typename xNonCVR<tOperator>::xUnit BinaryExponentiation(tOperator && Operator, typename xNonCVR<tOperator>::xUnit Base, size_t Times)
    {
        using xOperator = xNonCVR<tOperator>;
        using xUnit = xNonCVR<typename xOperator::xUnit>;
        static_assert(std::is_same_v<xUnit, typename xOperator::xUnit>);
        static_assert(std::is_same_v<xUnit, xNonCVR<decltype(xOperator::Unit)>>);

        auto Result = xOperator::Unit;
        while(Times) {
            if (Times & 0x01) {
                Result = std::forward<tOperator>(Operator)(Result, Base);
            }
            Base = std::forward<tOperator>(Operator)(Base, Base);
            Times >>= 1;
        }
        return Result;
    }

}
