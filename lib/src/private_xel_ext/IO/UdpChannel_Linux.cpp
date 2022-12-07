#include <xel_ext/IO/UdpChannel.hpp>

X_NS
{

	bool xUdpChannel::Init(xIoContext * IoContextPtr, iListener * ListenerPtr)
	{
		return false;
	}

	bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr)
	{
		return false;
	}

	void xUdpChannel::Clean()
	{
		assert(_Socket != InvalidSocket);
		XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
		X_DEBUG_RESET(_ListenerPtr);
	}





}
