#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/NetBase.hpp>
#include <zec_ext/IO/UdpChannel.hpp>
#include <zec/Util/Command.hpp>
#include <zec/Util/Chrono.hpp>
#include <zec/String.hpp>
#include <iostream>

using namespace zec;
using namespace std;

struct xServerListner
: xUdpChannel::iListener
{
	bool Stop = false;
	void OnError(xUdpChannel * ChannelPtr, const char * Message) override {
		cout << "Error:" << Message << endl;
	}
    void OnData(xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override
	{
		cout << "ServerDataReceived \n" << HexShow(DataPtr, DataSize, true) << endl;
		ChannelPtr->PostData(DataPtr, DataSize, RemoteAddress);

		if (std::string((const char *)DataPtr, DataSize) == "stop") {
			Stop = true;
		}
	}
};

void DoServer(const xNetAddress & Address)
{
	xIoContext IoContext;
	auto Guard = xResourceGuard(IoContext);

	xServerListner ServerListner;
	auto Channel = xUdpChannel();
	auto ChannelGuard = xResourceGuard(Channel, &IoContext, Address, &ServerListner);

	while(!ServerListner.Stop) {
		IoContext.LoopOnce(100);
	}
}

struct xClientListner
: xUdpChannel::iListener
{
	bool Data = false;
    void OnData(xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override
	{
		cout << "ClientDataReceived \n" << HexShow(DataPtr, DataSize, true) << endl;
		Data = true;
	}
};

void DoClient(const xNetAddress & Address, const std::string & Data)
{
	xIoContext IoContext;
	auto Guard = xResourceGuard(IoContext);

	xClientListner ClientListner;
	auto Channel = xUdpChannel();
	auto ChannelGuard = xResourceGuard(Channel, &IoContext, &ClientListner);

	Channel.PostData(Data.data(), Data.size(), Address);

	xTimer Timer;
	while(!ClientListner.Data) {
		if (Timer.TestAndTag(1s)) {
			cerr << "Timeout" << endl;
			break;
		}
		IoContext.LoopOnce(100);
	}
}

int main(int argc, char *argv[])
{
	auto CmdLine = xCommandLine { argc, argv, {
		{ 'c', "client", "client", false },
		{ 'a', "address", "address", true }
	}};

	auto OptAddress = CmdLine["address"];
	if (!OptAddress()) {
		cerr << "missing address" << endl;
		return -1;
	}
	auto Address = xNetAddress::Parse(*OptAddress);
	if (CmdLine["client"]()) {
		if (CmdLine.GetArgCount() < 2) {
			cerr << "missing messages" << endl;
			return -1;
		}
		DoClient(Address, CmdLine[1]);
		return 0;
	};
	DoServer(Address);
	return 0;
}
