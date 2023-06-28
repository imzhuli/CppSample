#include <iostream>
#include <xel/Common.hpp>
#include <xel/Byte.hpp>
#include <xel/String.hpp>
#include <xel/Queue.hpp>
#include <xel/Util/Command.hpp>

using namespace std;
using namespace xel;

struct xNode : xQueueNode
{
    int I = 0;
};

int main(int argc, char * argv[])
{
    auto Cmd = xCommandLine(argc, argv, {
    });

    for (auto Arg : Cmd.GetArgs()) {
        cout << "-- " << Arg << endl;
    }

    xNode N1, N2, N3;
    xQueue<xNode> Queue;
    Queue.Push(N1);
    Queue.Push(N2);
    Queue.Push(N3);

    int Counter = 0;
    for (auto & N : Queue) {
        N.I = ++Counter;
    }
    for (auto & N : Queue) {
        cout << N.I << endl;
    }
    while(auto NPtr = Queue.Pop())
    {
        cout << "Pop: " << NPtr->I << endl;
    }

    return 0;
}