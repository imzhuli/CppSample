#include <xel_ext/IO/TcpConnectionEx.hpp>

X_NS
{

	bool xAutoClientTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress ServerAddress, iListener * ListenerPtr)
	{
		assert(IoContextPtr);
		assert(ServerAddress);
		assert(ListenerPtr);

		_IoContextPtr   = IoContextPtr;
		_ServerAddress  = ServerAddress;
		_ListenerPtr    = ListenerPtr;

		_Active          = false;
		_Connected       = false;
		_Dying           = false;
		return true;
	}

	void xAutoClientTcpConnection::Clean()
	{
		if (Steal(_Active)) {
			_Connection.Clean();
			_Connected = false;
		}
		_Dying = false;

		_ListenerPtr   = nullptr;
		_ServerAddress = {};
		_IoContextPtr  = nullptr;
	}

	void xAutoClientTcpConnection::Check(uint64_t NowMS)
	{
		if (Steal(_Dying)) {
			_CheckTimestamp = NowMS;
			_Connection.Clean();
			_Active = false;
			_Connected = false;
			return;
		}
		if (_Connected) {
			return;
		}
		if (_Active && NowMS - _CheckTimestamp >= 5'000) { // connection timeout
			_CheckTimestamp = NowMS;
			_Connection.Clean();
			_Active = false;
			_Connected = false;
			_Dying = false;
			return;
		}
		// reconnect
		if (NowMS - _CheckTimestamp >= 15'000) { // reconnect timeout}
            _CheckTimestamp = NowMS;
			if (!_Connection.Init(_IoContextPtr, _ServerAddress, this)) {
				return;
			}
			_Connected = _Connection.IsConnected();
			_Active = true;
			++_Version;

            if (_Connected) {
                _ListenerPtr->OnConnected(this);
            }
            return;
		}
		return;
	}

	size_t xAutoClientTcpConnection::OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize)
	{
		return _ListenerPtr->OnData(this, DataPtr, DataSize);
	}

	size_t xAutoClientTcpConnection::PostData(const void * DataPtr, size_t DataSize)
	{
		if (!_Connected) {
			return 0;
		}
		return _Connection.PostData(DataPtr, DataSize);
	}

	void xAutoClientTcpConnection::OnConnected(xTcpConnection * TcpConnectionPtr)
	{
        X_DEBUG_PRINTF("xAutoClientTcpConnection::OnConnected\n");
		_Connected = true;
		_ListenerPtr->OnConnected(this);
	}

	void xAutoClientTcpConnection::OnPeerClose(xTcpConnection * TcpConnectionPtr)
	{
		_Connected = false;
		ResetConnection();
	}

}