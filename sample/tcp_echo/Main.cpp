#include <zec/Util/Command.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <iostream>

using namespace std;
using namespace zec;

int main(int argc, char *argv[])
{
    auto Cmd = xCommandLine{ argc, argv, {}};
    auto Tag = std::string{"TCP_ECHO"};

    if (Cmd.GetArgCount() > 1) {
        Tag = Cmd[1];
        cout << "UpdateTag: %s", Tag.c_str();
    }

    // xIoContext IoContext;
    // xTcpServer TcpServer;
    

    return 0;
}
