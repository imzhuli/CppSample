#include <zec_ext/IO/WebSocketPool.hpp>

ZEC_NS
{

    bool xWebSocketSessionPool::Init(xIoContext * IoContextPtr, const xNetAddress * AddressList, size_t AddressCount, const std::string & Origin, const std::string &Target, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(AddressCount);
        assert(_SessionTargets.empty());

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
        // TODO

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

}

