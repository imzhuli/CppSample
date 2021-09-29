#pragma once
#include "../Common.hpp"
#include <atomic>

ZEC_NS
{

	class xVersion
	{
	public:
		using xId = int64_t;

		ZEC_INLINE operator    xId() const { return _Version;  }
		ZEC_INLINE void        Set(xId ver) { _Version = ver; }
		ZEC_INLINE void        Upgrade() { ++_Version; }

		ZEC_INLINE bool        operator == (const xVersion & other) const { return _Version == other._Version; }
		ZEC_INLINE bool        operator != (const xVersion & other) const { return _Version != other._Version; }
		ZEC_INLINE bool        Synchronize(xId newVersion) { if (newVersion == _Version) { return false; } _Version = newVersion; return true; }

		ZEC_INLINE xVersion() = default;
		ZEC_INLINE xVersion(const xVersion &) = default;
		ZEC_INLINE ~xVersion() = default;

	private:
		xId _Version = 0;
	};

	class xVersionAtomic
	{
	public:
		using xId = int64_t;

		ZEC_INLINE operator    xId() const { return _Version;  }
		ZEC_INLINE void        Set(xId ver) { _Version = ver; }
		ZEC_INLINE void        Upgrade() { ++_Version; }

		ZEC_INLINE bool        operator == (const xVersionAtomic & other) const { return _Version == other._Version; }
		ZEC_INLINE bool        operator != (const xVersionAtomic & other) const { return _Version != other._Version; }
		ZEC_INLINE bool        Synchronize(xId newVersion) { return newVersion != _Version.exchange(newVersion); }

		ZEC_INLINE xVersionAtomic() = default;
		ZEC_INLINE xVersionAtomic(const xVersionAtomic & other) : _Version{ other._Version.load() } {}
		ZEC_INLINE ~xVersionAtomic() = default;

	private:
		std::atomic<xId> _Version = { 0 };
	};

}
