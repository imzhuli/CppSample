#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <zec/String.hpp>

#include <iostream>

using namespace zec;
using namespace std;

static bool Running = true;
static size_t ConnectionCount = 0;

class xTestListener
: public xTcpConnection::iListener
{
	virtual void   OnConnected(xTcpConnection * TcpConnectionPtr)  {
		cout << "NewConnection, Total Connections:" << ++ConnectionCount << endl;
	 }
	virtual size_t OnReceiveData(xTcpConnection * TcpConnectionPtr, const void * DataPtr, size_t DataSize) {
		cout << "OnReceiveData:" << endl;
		cout << HexShow(DataPtr, DataSize) << endl;


		return DataSize;
	}
	virtual void   OnPeerClose(xTcpConnection * TcpConnectionPtr)  {
		cout << "OnPeerClose" << endl;
		if (0 == --ConnectionCount){
			Running = false;
		}
	}
	virtual void   OnError(xTcpConnection * TcpConnectionPtr) {
		cout << "OnError" << endl;
		if (0 == --ConnectionCount){
			Running = false;
		}
	}
};

int main(int, char **)
{
	xIoContext IoContext;
	xTcpServer TcpServer;
	xTestListener Listener;

	IoContext.Init();
	TcpServer.Init(&IoContext, "0.0.0.0", 7788, &Listener);

	while(Running) {
		IoContext.LoopOnce(100);
	};

	TcpServer.Clean();
	IoContext.Clean();

	return 0;
}
