#include <xel_ext/Utility/Uuid.hpp>
#include <xel/String.hpp>
#include <iostream>
using namespace xel;
using namespace std;

int main(int, char **)
{
    ubyte s[16] = "hello world!";

    xUuid u0;
    xUuid u1{NoInit};
    xUuid u2{s};
    xUuid u3{GeneratorInit};

    u1 = u2;
    if (!(u1 == u2 && u0 < u1 && u0 < u2 && u0 < u3)) {
        return -1;
    }

    cout << StrToHex(u3.GetData(), u3.GetSize()) << endl;
    return 0;
}
