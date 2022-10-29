#include <xel/Common.hpp>
#include <xel/Algorithm/BinaryExponentiation.hpp>
#include <iostream>

using namespace xel;
using namespace std;

struct xIngralMultiplication
{
    using xUnit = intmax_t;
    static constexpr xUnit Unit = 1;

    xUnit operator()(const xUnit & L, const xUnit & R) const {
        return L * R;
    }
};

struct xFibonacciMultiplication
{
    using xPair = struct {
        uintmax_t Fn, Fn1;
    };
    using xUnit = struct {
        uintmax_t m00, m01;
        uintmax_t m10, m11;
    };
    static constexpr xUnit Unit = {
        1, 0,
        0, 1,
    };
    static constexpr xUnit Transform = {
        0, 1,
        1, 1,
    };

    xUnit operator()(const xUnit & L, const xUnit & R) const {
        return {
            L.m00 * R.m00 + L.m01 * R.m10, L.m00 * R.m01 + L.m01 * R.m11,
            L.m10 * R.m00 + L.m11 * R.m10, L.m10 * R.m01 + L.m11 * R.m11,
        };
    }
};

xFibonacciMultiplication::xPair operator * (const xFibonacciMultiplication::xPair & Pair, const xFibonacciMultiplication::xUnit & Transform) {
    return { Pair.Fn * Transform.m00 + Pair.Fn1 * Transform.m10, Pair.Fn * Transform.m01 + Pair.Fn1 * Transform.m11 };
}

int main(int argc, char *argv[])
{
    auto Result3 = BinaryExponentiation(xIngralMultiplication{}, 3, 13);
    cout << "Result3:" << Result3 << endl;

    for (size_t i = 0 ; i < 128; ++i) {
        auto F3 = xFibonacciMultiplication::xPair{ 1, 1 } * BinaryExponentiation(xFibonacciMultiplication{}, xFibonacciMultiplication::Transform, i);
        cout << "F_" << i << ": " << F3.Fn << endl;
    }
    return 0;
}

