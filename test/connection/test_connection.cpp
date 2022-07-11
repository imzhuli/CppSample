#include <zec/Common.hpp>
#include <zec/Util/Command.hpp>
#include <zec/Util/Chrono.hpp>
#include <zec_ext/IO/TcpConnectionPool.hpp>
#include <string>
#include <iostream>

using namespace zec;
using namespace std;

struct xTestLisetner
: xTcpConnectionPool::iListener
{
	std::string Text;
    size_t  OnData(xTcpConnectionPool * ConnectionPoolPtr, size_t Index, void * DataPtr, size_t DataSize) override
	{
		Text.append((const char *)DataPtr, DataSize);
		return DataSize;
	}
};

int main(int argc, char * argv[])
{
	auto CmdLine = zec::xCommandLine{ argc, argv, {
		{ 'a', "addr", "addr", true }
	}};

	auto OptIp = CmdLine["addr"];
	if (!OptIp()) {
		cerr << "Usage:" << endl;
		cerr << "test_connection --addr ip:port" << endl;
		return -1;
	}

	xIoContext IoContext;
	auto IoContextGuard = xResourceGuard(IoContext);

	xTestLisetner TestListener;

	xTcpConnectionPool Pool;
	auto PoolGuard = xResourceGuard(Pool, &IoContext, std::vector{
		xNetAddress::Parse(*OptIp),
		xNetAddress::Parse(*OptIp)
	}, &TestListener);
	Pool.Check(GetMilliTimestamp());

	std::string Request = "GET / HTTP/1.1\r\nUser-Agent: zec-test/1.0\r\n\r\n";
	cout << "Request:" << endl;
	cout << Request << endl;
	Pool.PostData(Request.data(), Request.length());
	Pool.PostData(Request.data(), Request.length());

	xTimer Timer;
	while(!Timer.TestAndTag(3s)) {
		IoContext.LoopOnce(100);
		Pool.Check(GetMilliTimestamp());
	}
	cout << TestListener.Text << endl;

	return 0;
}
