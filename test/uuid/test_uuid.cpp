#include <zec_ext/Utility/Uuid.hpp>
#include <iostream>
using namespace zec;
using namespace std;

int main(int, char **)
{
    ubyte s[16] = "hello world!";
    xUuid u1;
    xUuid u2{s};
    xUuid u3{GeneratorInit};

    cout << u2.GetData() << endl;

    u1 = u2;
    cout << u1.GetData() << endl;

    cout << u3.GetData() << endl;

    cout << "equal? " << YN(u1 == u2) << endl;

    return 0;
}
