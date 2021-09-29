#pragma once
#include "../Common.hpp"
#include <mutex>

ZEC_NS
{

	template<typename tMutex, typename tFuncObj, typename ... tArgs>
	auto SyncCall(tMutex && Mutex, tFuncObj && Func, tArgs && ... Args)
	{
		auto Guard = std::lock_guard(std::forward<tMutex>(Mutex));
		return std::forward<tFuncObj>(Func)(std::forward<tArgs>(Args)...);
	}

}
