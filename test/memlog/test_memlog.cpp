#include <zec/Util/Logger.hpp>
#include <iostream>

using namespace zec;
using namespace std;

int main(int argc, char * argv[])
{
    xMemoryLogger Logger;
    Logger.Init(128, 15);

    for (int i = 0 ; i < 65536; ++i) {
        Logger.I("Hello, this is log line %d", i);
    }
    Logger.Output();

    Logger.Clean();
	return 0;
}