#include <iostream>
#include <zec/Byte.hpp>
#include <zec/String.hpp>

using namespace std;
using namespace zec;

int main(int, char **)
{
    uint64_t Version = 2'2'0105'111'01UL;

    cout << "Version: " << Version << endl;
    cout << "V32: " << (uint32_t)Version << endl;

    auto Guard = []{
        auto const Guard = xScopeGuard([]{
            cout << "!Guard!!!" << endl;
        });
        cout << "InScope" << endl;
        return Guard;
    }();

    ubyte Buffer[1024];
    auto W = xStreamWriter(Buffer);
    auto R = xStreamReader(Buffer);

    (void)W;
    (void)R;
    return 0;
}