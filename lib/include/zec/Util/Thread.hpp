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

	template<typename T>
	class iAsyncUpdater
	: xNonCopyable
	{
		static_assert(!std::is_reference_v<T>);
	public:
		struct xInstanceWrapper {
			T *       InstancePtr;
			uint64_t  ExpireMS;
		};
		virtual xInstanceWrapper   SyncNewInstance() = 0;
		virtual void               SyncRetainInstance(T * TargetPtr) = 0;  // Target is never nullptr;
		virtual void               SyncReleaseInstance(T * TargetPtr) = 0; // Target is never nullptr;
	};

	template<typename T>
	class xAsyncTimer final
	: xNonCopyable
	{
		static_assert(!std::is_reference_v<T>);
		using xObject = T;
		using iUpdater = iAsyncUpdater<xObject>;
	public:
		ZEC_INLINE xAsyncTimer(iUpdater * UpdaterPtr, uint64_t ExpireMinMS = 1000, uint64_t ExpireMaxMS = 60000)
		: _UpdaterPtr(UpdaterPtr)
		, _ExpireMinMS(ExpireMinMS)
		, _ExpireMaxMS(std::max(ExpireMinMS, ExpireMaxMS))
		, _ExpireMS(_ExpireMinMS)
		{
			assert(_UpdaterPtr);
		}
		ZEC_INLINE ~xAsyncTimer() {
			if(_RunLock.load() != NO_RUN_MASK) {
				Error();
			}
		}
		ZEC_INLINE bool Init() {
			uint32_t NoRun = NO_RUN_MASK;
			if (!_RunLock.compare_exchange_strong(NoRun, RUN_MASK)) {
				return false;
			}
			_SyncThread = std::thread{[this]{
				xObject * RemoveInfoPtr = nullptr;
				do {
					if (_ObjectPtr && !_RefreshTimer.TestAndTag(xMilliSeconds{_ExpireMS})) {
						std::this_thread::sleep_for(xMilliSeconds{_ExpireMinMS});
						continue;
					}
					auto NewInstance = _UpdaterPtr->SyncNewInstance();
					_ExpireMS = std::min(NewInstance.ExpireMS, _ExpireMaxMS);

					RemoveInfoPtr = nullptr;
					if (_ObjectPtr != NewInstance.InstancePtr) {
						auto Guard = xSpinlockGuard{ _Spinlock };
						RemoveInfoPtr = Steal(_ObjectPtr, NewInstance.InstancePtr);
					}
					if (RemoveInfoPtr) {
						_UpdaterPtr->SyncReleaseInstance(RemoveInfoPtr);
					}

				} while(RUN_MASK == _RunLock.load());

				do {
					auto Guard = xSpinlockGuard{ _Spinlock };
					RemoveInfoPtr = Steal(_ObjectPtr);
				} while(false);
				if (RemoveInfoPtr) {
					_UpdaterPtr->SyncReleaseInstance(RemoveInfoPtr);
				}
			}};
			return true;
		}
		ZEC_INLINE void Clean() {
			uint32_t RunningExpected = RUN_MASK;
			if (!_RunLock.compare_exchange_strong(RunningExpected, INIT_MASK)) {
				Error("Invalid Async status");
				return;
			}
			_SyncThread.join();
			Reset(_SyncThread);
			_RunLock.store(NO_RUN_MASK);
		}
		ZEC_INLINE xObject * GetInstance() {
			auto Guard = xSpinlockGuard{ _Spinlock };
			if (_ObjectPtr) {
				_UpdaterPtr->SyncRetainInstance(_ObjectPtr);
			}
			return _ObjectPtr;
		}
	private:
		static constexpr uint32_t NO_RUN_MASK   = 0x0000;
		static constexpr uint32_t INIT_MASK     = 0x0100;
		static constexpr uint32_t RUN_MASK      = 0x0101;

		std::atomic<uint32_t>    _RunLock;
		std::thread              _SyncThread;
		xTimer                   _RefreshTimer;
		xSpinlock                _Spinlock    {};

		xObject *                _ObjectPtr   {};
		iUpdater *               _UpdaterPtr  {};
		uint64_t                 _ExpireMinMS {};
		uint64_t                 _ExpireMaxMS {};
		uint64_t                 _ExpireMS    {};
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
