#pragma once
#include "../Common.hpp"
#include <chrono>
#include <ctime>

ZEC_NS
{

	using xHiResxClock = std::chrono::high_resolution_clock;
	using xHiResxTimePoint = xHiResxClock::time_point;
	using xHiResDuration = xHiResxClock::duration;

	using xSteadyxClock = std::conditional<xHiResxClock::is_steady, xHiResxClock, std::chrono::steady_clock>::type;
	using xSteadyxTimePoint = xSteadyxClock::time_point;
	using xSteadyDuration = xSteadyxClock::duration;
	using namespace std::chrono_literals;

	using xSeconds        = std::chrono::seconds;
	using xMilliSeconds   = std::chrono::milliseconds;
	using xMicroSeconds   = std::chrono::microseconds;
	using xNanoSeconds    = std::chrono::nanoseconds;

#if __cplusplus < 202002L
	ZEC_API uint64_t GetMicroTimestamp();
#else
	ZEC_INLINE uint64_t GetMicroTimestamp() {
		return std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();
	}
#endif
	ZEC_INLINE uint64_t GetMilliTimestamp() { return GetMicroTimestamp() / 1000; }
	ZEC_INLINE uint64_t GetTimestamp()      { return GetMicroTimestamp() / 1000000; }

	class xTimer
	{
	public:
		using xClock = xSteadyxClock;
		using xTimePoint = xSteadyxTimePoint;
		using xDuration = xSteadyDuration;
		static_assert(std::is_signed_v<xDuration::rep>);

		ZEC_STATIC_INLINE xTimePoint Now() {
			return xClock::now();
		}

	private:
		xTimePoint _LastTagTime;

	public:
		ZEC_INLINE xTimer() {
			_LastTagTime = Now();
		}

		ZEC_INLINE xDuration Elapsed() {
			return Now() - _LastTagTime;
		}

		ZEC_INLINE xDuration Skip(const xDuration & duration)
		{
			_LastTagTime += duration;
			return duration;
		}

		ZEC_INLINE void Tag() {
			Tag(Now());
		}

		ZEC_INLINE void Tag(xTimePoint tp) {
			_LastTagTime = tp;
		}

		ZEC_INLINE xTimePoint TagAndGet() {
			auto N = Now();
			Tag(N);
			return N;
		}

		ZEC_INLINE xDuration TagAndGetElapsed() {
			auto L = _LastTagTime;
			auto N = Now();
			Tag(N);
			return N - L;
		}

		ZEC_INLINE bool TestAndTag(xDuration testDuration) {
			xTimePoint n = Now();
			if (Diff(n - _LastTagTime, testDuration).count() >= 0) {
				_LastTagTime = n;
				return true;
			}
			return false;
		}

		ZEC_INLINE xTimePoint GetAndTag() {
			return Steal(_LastTagTime, Now());
		}

		/**
		 * @brief Consume:
		 *  Consume time by duration, return true if consuming is executed
		 *
		 *  Usage:
		 *  Sometimes, we expect no matter how much test is delayed,
		 *  some procedure must take place as if test happens continously, and by interval and no event is missed. (ex: physics engine)
		 *
		 *  Sample:
		 *  while(Consume(1s)) {
		 * 		// do something.
		 *  }
		 *  so even if the test happens 10s later, it acts as if test happeds every 1s, and 10 times in all.
		 */
		ZEC_INLINE bool Consume(xDuration duration) {
			xTimePoint target = _LastTagTime + duration;
			if (Diff(Now(), target).count() >= 0) {
				_LastTagTime = target;
				return true;
			}
			return false;
		}
	};

} // namespace ze
