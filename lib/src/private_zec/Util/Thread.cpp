#include <zec/Util/Thread.hpp>

ZEC_NS
{

	void xThreadSynchronizer::Aquire()
	{
		std::unique_lock lock(_Mutex);
		++_TotalSize;
	}

	void xThreadSynchronizer::Release()
	{
		std::unique_lock lock(_Mutex);
		auto & context = _Coutnexts[_ActiveContext];
		if (--_TotalSize == context.xWaitingCount && _TotalSize != 0) {
			context.xWaitingCount = 0;
			std::swap(_ActiveContext, _OtherContext);
			lock.unlock();
			context.xCondtion.notify_all();
		}
	}

	void xThreadSynchronizer::Sync()
	{
		std::unique_lock lock(_Mutex);
		auto & context = _Coutnexts[_ActiveContext];
		if (++context.xWaitingCount == _TotalSize) {
			context.xWaitingCount = 0;
			std::swap(_ActiveContext, _OtherContext);
			lock.unlock();
			context.xCondtion.notify_all();
		}
		else {
			context.xCondtion.wait(lock, [&context] { return context.xWaitingCount == 0; } );
		}
	}

}