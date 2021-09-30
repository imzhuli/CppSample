#pragma once
#include "../Common.hpp"
#include "./Chrono.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

ZEC_NS
{

	class xSpinlock final
	{
	public:
		ZEC_INLINE void Lock() const noexcept {
			for (;;) {
				// Optimistically assume the lock is free on the first try
				if (!_LockVariable.exchange(true, std::memory_order_acquire)) {
					return;
				}
				// Wait for lock to be released without generating cache misses
				while (_LockVariable.load(std::memory_order_relaxed)) {
					// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
					// hyper-threads
					// gcc/clang: __builtin_ia32_pause();
					// msvc: _mm_pause();
				}
			}
		}

		ZEC_INLINE bool TryLock() const noexcept {
			// First do a relaxed load to check if lock is free in order to prevent
			// unnecessary cache misses if someone does while(!try_lock())
			return !_LockVariable.load(std::memory_order_relaxed) &&
				!_LockVariable.exchange(true, std::memory_order_acquire);
		}

		ZEC_INLINE void Unlock() const noexcept {
			_LockVariable.store(false, std::memory_order_release);
		}
	private:
		mutable std::atomic<bool> _LockVariable = {0};
	};

	class xSpinlockGuard final
	: xNonCopyable
	{
	public:
		[[nodiscard]] ZEC_INLINE xSpinlockGuard(const xSpinlock & Spinlock)
		: _Spinlock(&Spinlock) {
			_Spinlock->Lock();
		}
		ZEC_INLINE ~xSpinlockGuard() {
			_Spinlock->Unlock();
		}
	private:
		const xSpinlock * _Spinlock;
	};

	class xThreadSynchronizer final
	{
	private:
		struct Context {
			int_fast32_t               xWaitingCount = 0;
			std::condition_variable    xCondtion;
		};

		std::mutex                 _Mutex;
		ssize32_t                  _TotalSize        = 0;
		uint_fast8_t               _ActiveContext    = 0;
		uint_fast8_t               _OtherContext     = 1;
		Context                    _Coutnexts[2];

	public:
		ZEC_API_MEMBER void Aquire();
		ZEC_API_MEMBER void Release();
		ZEC_API_MEMBER void Sync();
	};

	class xThreadChecker final
	{
	public:
		ZEC_INLINE void Init()  { _ThreadId = std::this_thread::get_id(); }
		ZEC_INLINE void Clean() { _ThreadId = {}; }
		ZEC_INLINE void Check() { if (std::this_thread::get_id() != _ThreadId) { Error(); }};
	private:
		std::thread::id _ThreadId;
	};

#ifndef NDEBUG
	using xDebugThreadChecker = xThreadChecker;
#else
	class xDebugThreadChecker
	{
	public:
		ZEC_INLINE void Init()  {}
		ZEC_INLINE void Clean() {}
		ZEC_INLINE void Check() {}
	};
#endif

}
