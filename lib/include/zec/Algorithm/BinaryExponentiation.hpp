#pragma once
#include "../Common.hpp"

ZEC_NS
{

    template<typename tOperator>
    ZEC_INLINE typename xNoCVR<tOperator>::xUnit BinaryExponentiation(tOperator && Operator, typename xNoCVR<tOperator>::xUnit Base, size_t Times)
    {
        using xOperator = xNoCVR<tOperator>;
        using xUnit = xNoCVR<typename xOperator::xUnit>;
        static_assert(std::is_same_v<xUnit, typename xOperator::xUnit>);
        static_assert(std::is_same_v<xUnit, xNoCVR<decltype(xOperator::Unit)>>);

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
