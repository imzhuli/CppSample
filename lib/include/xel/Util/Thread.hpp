#pragma once
#include "../Common.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

X_NS
{

	template<typename tMutex, typename tFuncObj, typename ... tArgs>
	auto SyncCall(tMutex && Mutex, tFuncObj && Func, tArgs && ... Args)
	{
		auto Guard = std::lock_guard(std::forward<tMutex>(Mutex));
		return std::forward<tFuncObj>(Func)(std::forward<tArgs>(Args)...);
	}

	class xSpinlock final
	{
	public:
		X_INLINE void Lock() const noexcept {
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

		X_INLINE bool TryLock() const noexcept {
			// First do a relaxed load to check if lock is free in order to prevent
			// unnecessary cache misses if someone does while(!try_lock())
			return !_LockVariable.load(std::memory_order_relaxed) &&
				!_LockVariable.exchange(true, std::memory_order_acquire);
		}

		X_INLINE void Unlock() const noexcept {
			_LockVariable.store(false, std::memory_order_release);
		}
	private:
		mutable std::atomic<bool> _LockVariable = {0};
	};

	class xSpinlockGuard final
	: xNonCopyable
	{
	public:
		[[nodiscard]] X_INLINE xSpinlockGuard(const xSpinlock & Spinlock)
		: _Spinlock(&Spinlock) {
			_Spinlock->Lock();
		}
		X_INLINE ~xSpinlockGuard() {
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
		X_API_MEMBER void Aquire();
		X_API_MEMBER void Release();
		X_API_MEMBER void Sync();
	};

	namespace __detail__
	{
		template<bool AutoReset = false>
		struct xEvent
		: xNonCopyable
		{
		private:
			std::mutex _Mutex;
			std::condition_variable _ConditionVariable;
			bool _Ready = false;

		public:
			X_INLINE void Reset() {
				auto Lock = std::unique_lock(_Mutex);
				_Ready = false;
			}

			template<typename tFuncObj>
			auto SyncCall(const tFuncObj & func) {
				auto Lock = std::unique_lock(_Mutex);
				return func();
			}

			template<typename tFuncPre, typename tFuncPost>
			X_INLINE void Wait(const tFuncPre & funcPre, const tFuncPost & funcPost) {
				auto Lock = std::unique_lock(_Mutex);
				funcPre();
				_ConditionVariable.wait(Lock, [this](){return _Ready;});
				if constexpr (AutoReset) {
					_Ready = false;
				}
				// Notice : the Post function is called after auto reset,
				// just incase it throws exception;
				funcPost();
			}
			template<typename tFuncPost = xPass>
			void Wait(const tFuncPost & funcPost = {}) {
				auto Lock = std::unique_lock(_Mutex);
				_ConditionVariable.wait(Lock, [this](){return _Ready;});
				if constexpr (AutoReset) {
					_Ready = false;
				}
				// Notice : the Post function is called after auto reset,
				// just incase it throws exception;
				funcPost();
			}

			template<typename Rep, typename Period, typename tFuncPre, typename tFuncPost>
			X_INLINE bool WaitFor(const std::chrono::duration<Rep, Period>& RelTime
					, const tFuncPre & funcPre,  const tFuncPost & funcPost) {
				auto Lock = std::unique_lock(_Mutex);
				funcPre();
				if (!_ConditionVariable.wait_for(Lock, RelTime, [this](){return _Ready;})) {
					return false;
				}
				if constexpr (AutoReset) {
					_Ready = false;
				}
				// Notice : the Post function is called after auto reset,
				// just incase it throws exception;
				funcPost();
				return true;
			}
			template<typename Rep, typename Period, typename tFuncPost = xPass>
			X_INLINE bool WaitFor(const std::chrono::duration<Rep, Period>& RelTime, const tFuncPost & funcPost = {}) {
				auto Lock = std::unique_lock(_Mutex);
				if (!_ConditionVariable.wait_for(Lock, RelTime, [this](){return _Ready;})) {
					return false;
				}
				if constexpr (AutoReset) {
					_Ready = false;
				}
				// Notice : the Post function is called after auto reset,
				// just incase it throws exception;
				funcPost();
				return true;
			}

			template<typename tFuncObj = xPass>
			X_INLINE std::enable_if_t<std::is_same_v<void, std::invoke_result_t<tFuncObj>>> Notify(const tFuncObj & PreNotifyFunc = {}) {
				do {
					auto Lock = std::unique_lock(_Mutex);
					PreNotifyFunc();
					_Ready = true;
					} while(false);
				_ConditionVariable.notify_one();
			}

			template<typename tFuncObj = xPass>
			X_INLINE std::enable_if_t<std::is_same_v<void, std::invoke_result_t<tFuncObj>>> NotifyAll(const tFuncObj & PreNotifyFunc = {}) {
				do {
					auto Lock = std::unique_lock(_Mutex);
					PreNotifyFunc();
					_Ready = true;
				} while(false);
				_ConditionVariable.notify_all();
			}
		};
	}
	using xEvent = __detail__::xEvent<false>;
	using xAutoResetEvent = __detail__::xEvent<true>;

	class xThreadChecker final
	{
	public:
		X_INLINE void Init()  { _ThreadId = std::this_thread::get_id(); }
		X_INLINE void Clean() { _ThreadId = {}; }
		X_INLINE void Check() { if (std::this_thread::get_id() != _ThreadId) { Error(); }};
	private:
		std::thread::id _ThreadId;
	};

#ifndef NDEBUG
	using xDebugThreadChecker = xThreadChecker;
#else
	class xDebugThreadChecker
	{
	public:
		X_INLINE void Init()  {}
		X_INLINE void Clean() {}
		X_INLINE void Check() {}
	};
#endif

}
