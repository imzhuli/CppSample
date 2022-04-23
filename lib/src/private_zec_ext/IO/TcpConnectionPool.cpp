#include <zec_ext/IO/TcpConnectionPool.hpp>

ZEC_NS
{
	bool xTcpConnectionPool::Init(xIoContext * IoContextPtr, const std::vector<xNetAddress>& Addresses, iListener * ListenerPtr)
	{
		auto AddressesTemp = Addresses;
		return Init(IoContextPtr, std::move(AddressesTemp), ListenerPtr);
	}

	bool xTcpConnectionPool::Init(xIoContext * IoContextPtr, std::vector<xNetAddress>&& Addresses, iListener * ListenerPtr)
	{
		assert(ListenerPtr);
		assert(IoContextPtr);
		assert(Addresses.size());

		assert(!_IoContextPtr);
		assert(!_Addresses.size());
		assert(!_Connections);
		assert(!_ListenerPtr);
		for (auto & Address : Addresses) {
			if (!Address) {
				return false;
			}
		}

		size_t TotalConnections = Addresses.size();
		_IoContextPtr = IoContextPtr;
		_Addresses = std::move(Addresses);
		_Connections = new xTcpConnectionEx[TotalConnections];
		for (size_t i = 0 ; i < TotalConnections; ++i) {
			auto & Connection = _Connections[i];
			Connection.ConnectionId = (uint64_t)i;
			_DisabledConnectionList.AddTail(Connection);
		}
		_ListenerPtr = ListenerPtr;
		return true;
	}

	void xTcpConnectionPool::Clean()
	{
		size_t TotalConnections = _Addresses.size();
		for (size_t i = 0 ; i < TotalConnections; ++i) {
			auto & Connection = _Connections[i];
			if (Connection.IsActive()) {
				Connection.Clean();
			}
		}
		delete [] Steal(_Connections);
		_Addresses.clear();
		_IoContextPtr = nullptr;
		_ListenerPtr = nullptr;
	}

	void xTcpConnectionPool::Check(uint64_t NowMS)
	{
		for (auto & Iter : _KillConnectionList) {
			auto & Connection = static_cast<xTcpConnectionEx &>(Iter);
			Connection.Clean();
			Connection.ConnectionTimestampMS = NowMS;
			_DisabledConnectionList.GrabTail(Connection);
		}

		uint64_t KillTimepoint = NowMS - _ReconnectTimeoutMS;
		xTcpConnectionExList TempList; // using templist to prevent infinate
		for (auto & Iter : _DisabledConnectionList) {
			auto & Connection = static_cast<xTcpConnectionEx &>(Iter);
			if (Connection.ConnectionTimestampMS > KillTimepoint) {
				break;
			}
			if (!Connection.Init(_IoContextPtr, _Addresses[Connection.ConnectionId], this)) {
				Connection.ConnectionTimestampMS = NowMS;
				TempList.GrabTail(Connection);
			} else {
				_EnabledConnectionList.GrabTail(Connection);
			}
		}
		_DisabledConnectionList.GrabListTail(TempList);
	}

	void xTcpConnectionPool::OnConnected(xTcpConnection * TcpConnectionPtr)
	{}

	bool xTcpConnectionPool::PostData(const void * DataPtr, size_t DataSize)
	{
		auto FirstConnectionPtr = static_cast<xTcpConnectionEx*>(_EnabledConnectionList.Head());
		if (!FirstConnectionPtr) {
			return false;
		}
		if (DataSize != FirstConnectionPtr->PostData(DataPtr, DataSize)) {
			Kill(FirstConnectionPtr);
			return false;
		}
		_EnabledConnectionList.GrabTail(*FirstConnectionPtr);
		return true;
	}

    size_t xTcpConnectionPool::OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize)
	{
		return _ListenerPtr->OnData(this, DataPtr, DataSize);
	}

}
