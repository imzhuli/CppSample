#include <zec_ext/IO/WebSocketPool.hpp>

ZEC_NS
{

    bool xWebSocketSessionPool::Init(xIoContext * IoContextPtr, const xNetAddress * AddressList, size_t AddressCount, const std::string & Origin, const std::string &Target, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(AddressCount);
        assert(_SessionTargets.empty());

        _IoContextPtr = IoContextPtr;
        _Origin = Origin;
        _Target = Target;
        _ListenerPtr = ListenerPtr;

        _SessionTargets.resize(AddressCount);
        for (size_t i = 0 ; i < AddressCount; ++i) {
            auto & Target = _SessionTargets[i];
            Target.Address = AddressList[i];
            Target.SessionPtr = new xWebSocketSessionEx();
            Target.SessionPtr->SessionId = (uint64_t)i;
            _DisabledSessionList.AddTail(*Target.SessionPtr);
        }
        return true;
    }

    void xWebSocketSessionPool::Clean()
    {
        for (auto & Iter : _SessionTargets) {
            auto & Target = static_cast<xSessionTarget&>(Iter);
            if (Target.SessionPtr->IsActive()) {
                Target.SessionPtr->Clean();
            }
        }
        Reset(_SessionTargets);
		Reset(_ListenerPtr);
		Reset(_IoContextPtr);
        Reset(_Origin);
        Reset(_Target);
    }

	void xWebSocketSessionPool::Check(uint64_t NowMS)
	{
		for (auto & Iter : _KillSessionList) {
			auto & Session = static_cast<xWebSocketSessionEx &>(Iter);
			Session.Clean();
			Session.SessionTimestampMS = NowMS;
			_DisabledSessionList.GrabTail(Session);
		}

		uint64_t KillTimepoint = NowMS - _ReconnectTimeoutMS;
		xWebSocketSessionList TempList; // using templist to prevent infinate
		for (auto & Iter : _DisabledSessionList) {
			auto & Session = static_cast<xWebSocketSessionEx &>(Iter);
			if (Session.SessionTimestampMS > KillTimepoint) {
				break;
			}
			if (!Session.Init(_IoContextPtr, _SessionTargets[Session.SessionId].Address, _Origin, _Target, this)) {
				Session.SessionTimestampMS = NowMS;
				TempList.GrabTail(Session);
			} else {
				_EnabledSessionList.GrabTail(Session);
			}
		}
		_DisabledSessionList.GrabListTail(TempList);
	}

    void xWebSocketSessionPool::OnWSMessage(xWebSocketSession * WebSocketClientPtr, bool Binary, void * DataPtr, size_t DataSize)
    {
        _ListenerPtr->OnWSMessage(this, Binary, DataPtr, DataSize);
    }

    void xWebSocketSessionPool::OnWSClose(xWebSocketSession * WebSocketClientPtr)
    {
        Kill(static_cast<xWebSocketSessionEx*>(WebSocketClientPtr));
    }

    bool xWebSocketSessionPool::PostTextData(const std::string_view & Data)
    {
        auto SessionPtr = static_cast<xWebSocketSessionEx*>(_EnabledSessionList.Head());
        if (!SessionPtr) {
            return false;
        }
        if (!SessionPtr->PostTextData(Data)) {
            Kill(SessionPtr);
            return false;
        }
        _EnabledSessionList.GrabTail(*SessionPtr);
        return true;
    }

    bool xWebSocketSessionPool::PostBinaryData(const void * DataPtr, size_t DataSize)
    {
        auto SessionPtr = static_cast<xWebSocketSessionEx*>(_EnabledSessionList.Head());
        if (!SessionPtr) {
            return false;
        }
        if (!SessionPtr->PostBinaryData(DataPtr, DataSize)) {
            Kill(SessionPtr);
            return false;
        }
        _EnabledSessionList.GrabTail(*SessionPtr);
        return true;
    }

}
