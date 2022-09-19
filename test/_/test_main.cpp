#include <iostream>
#include <zec/Byte.hpp>
#include <zec/String.hpp>

using namespace std;
using namespace zec;

static bool Exit = false;

int main(int, char **)
{
    uint64_t Version = 2'2'0105'111'01UL;

    cout << "Version: " << Version << endl;
    cout << "V32: " << (uint32_t)Version << endl;

    auto Guard = []{
        auto Guard = xScopeGuard([]{
            if (Steal(Exit, true)) {
                Error("Dup exit");
            }
            cout << "!Guard!!!" << endl;
        });
        cout << "InScope" << endl;
        return Guard;
    }();

    auto Guard1 = std::move(Guard);
    auto Guard2 = std::move(Guard);

    ubyte Buffer[1024];
    auto W = xStreamWriter(Buffer);
    auto R = xStreamReader(Buffer);

    (void)W;
    (void)R;
    return 0;
}