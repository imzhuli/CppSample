#pragma once
#include "../Common.hpp"
#include <atomic>

X_NS
{

	class xVersion
	{
	public:
		using xId = uint64_t;

		X_INLINE operator    xId() const { return _Version;  }
		X_INLINE void        Set(xId ver) { _Version = ver; }
		X_INLINE void        Upgrade() { ++_Version; }

		X_INLINE bool        operator == (const xVersion & other) const { return _Version == other._Version; }
		X_INLINE bool        operator != (const xVersion & other) const { return _Version != other._Version; }
		X_INLINE bool        Synchronize(xId newVersion) { if (newVersion == _Version) { return false; } _Version = newVersion; return true; }

		X_INLINE xVersion() = default;
		X_INLINE xVersion(const xVersion &) = default;
		X_INLINE ~xVersion() = default;

	private:
		xId _Version = 0;
	};

	class xVersionAtomic
	{
	public:
		using xId = uint64_t;

		X_INLINE operator    xId() const { return _Version;  }
		X_INLINE void        Set(xId ver) { _Version = ver; }
		X_INLINE void        Upgrade() { ++_Version; }

		X_INLINE bool        operator == (const xVersionAtomic & other) const { return _Version == other._Version; }
		X_INLINE bool        operator != (const xVersionAtomic & other) const { return _Version != other._Version; }
		X_INLINE bool        Synchronize(xId newVersion) { return newVersion != _Version.exchange(newVersion); }

		X_INLINE xVersionAtomic() = default;
		X_INLINE xVersionAtomic(const xVersionAtomic & other) : _Version{ other._Version.load() } {}
		X_INLINE ~xVersionAtomic() = default;

	private:
		std::atomic<xId> _Version = { 0 };
	};

}
