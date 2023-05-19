#include <iostream>
#include <xel/Common.hpp>
#include <xel/Byte.hpp>
#include <xel/String.hpp>
#include <xel/Util/Command.hpp>

using namespace std;
using namespace xel;

int main(int argc, char * argv[])
{
    auto Cmd = xCommandLine(argc, argv, {
    });

    for (auto Arg : Cmd.GetArgs()) {
        cout << "-- " << Arg << endl;
    }

    return 0;
}