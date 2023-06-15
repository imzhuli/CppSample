#pragma once
#include "../Common.hpp"
#include <random>

X_NS
{

	class xIndexId;
	template<bool RandomKey = false>
	class xIndexIdPool;
	template<typename tValue, bool RandomKey = false>
	class xIndexedStorage;

	/***
	 * @name xIndexId
	 * @brief an 64 bit integer, with lower 32 bits representing an Index
	 *        and the higher 32 bit with random value as a check key
	 * @note
	 *    for pooled use,
	 *      the highest 2 bits in Index part and is always zero,
	 *      the second highest bit in Key part is always one (aka: KeyInUseBitmask == 0x4000'0000u),
	 *      so the id pool could use one 32bit integer to store an index to next free node in chain
	 *      while the index itself is free, or a key value with KeyInUseBitmask set while the index is in use;
	 *    Especially, since all allocated index has KeyInUseBitmask set to 1, a valid index id is never zero;
	 *    The highest bit of Key is always zero, so that a valid key is never (uint32_t)-1, which matches NoFreeIndex indicator in id pool,
	 *    KeyMask is used to remove the second highest bit of the key, so that key never matches NoFreeIndex, which might be used for attacking
	 * */
	class xIndexId final
	{
	public:
		X_INLINE xIndexId() = default;
		X_INLINE constexpr xIndexId(uint64_t Value) : _Value(Value) {};
		X_INLINE constexpr operator uint64_t () const { return _Value; }
		X_INLINE constexpr uint64_t operator ()() const { return _Value; }

		X_INLINE uint32_t GetIndex() const { return static_cast<uint32_t>(_Value); }
		X_INLINE uint32_t GetKey()   const { return static_cast<uint32_t>(_Value >> 32);}

		static constexpr uint64_t InvalidValue  = static_cast<uint64_t>(0);

	private:
		uint64_t _Value;

		template<bool RandomKey>
		friend class xIndexIdPool;
		template<typename tValue, bool RandomKey>
		friend class xIndexedStorage;

		X_API_STATIC_MEMBER uint32_t TimeSeed();
		static constexpr const uint32_t MaxIndexValue    = ((uint32_t)0x3FFF'FFFFu);
		static constexpr const uint32_t KeyInUseBitmask  = ((uint32_t)0x4000'0000u);
		static constexpr const uint32_t KeyMask          = MaxIndexValue | KeyInUseBitmask;
		X_STATIC_INLINE bool IsSafeKey(uint32_t Key) { return X_LIKELY(Key != (uint32_t)-1); }
	};

	template<bool RandomKey>
	class xIndexIdPool final
	: xNonCopyable
	{
	public:
		bool Init(size_t Size)
		{
			assert(Size <= xIndexId::MaxIndexValue);
			assert(_IdPoolPtr == nullptr);
			assert(_NextFreeIdIndex == NoFreeIndex);
			assert(_InitedId == 0);

			_IdPoolPtr = new uint32_t[Size];
			_IdPoolSize = (size32_t)Size;
			_InitedId = 0;
			_NextFreeIdIndex = NoFreeIndex;

		    _Counter = xIndexId::TimeSeed();
			_Random32.seed(_Counter);
			return true;
		}

		X_INLINE std::enable_if_t<!RandomKey, bool> DebugInit(size_t Size, uint32_t InitCounter = 0) {
			if(!Init(Size)) {
				return false;
			}
			_Counter = InitCounter;
			return true;
		}

		void Clean()
		{
			assert(_IdPoolPtr);
			delete [] Steal(_IdPoolPtr);
			_InitedId = 0;
			_IdPoolSize = 0;
			_NextFreeIdIndex = NoFreeIndex;
		}

		X_INLINE xIndexId Acquire() {
			uint32_t Index;
			if (_NextFreeIdIndex == NoFreeIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				Index = _InitedId++;
			} else {
				Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex]);
			}
			uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
			Rand |= xIndexId::KeyInUseBitmask;
			Rand &= xIndexId::KeyMask;
			_IdPoolPtr[Index] = Rand;
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		X_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
		}

		X_INLINE bool Check(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			return X_LIKELY(xIndexId::IsSafeKey(Key)) && X_LIKELY(Key == _IdPoolPtr[Index]);
		}

		X_INLINE bool CheckAndRelease(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			if(!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == _IdPoolPtr[Index])) {
				return false;
			}
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
			return true;
		}

	private:
		size32_t      _InitedId   = 0;
		size32_t      _IdPoolSize = 0;
		uint32_t*     _IdPoolPtr  = nullptr;
		uint32_t      _NextFreeIdIndex = NoFreeIndex;
		uint32_t      _Counter = 0;
		std::mt19937  _Random32;
		static constexpr const uint32_t CounterStep = 1;
		static constexpr const uint32_t NoFreeIndex = static_cast<uint32_t>(-1);
	};

	template<typename tValue, bool RandomKey>
	class xIndexedStorage final
	: xNonCopyable
	{
		static_assert(!std::is_reference_v<tValue> && !std::is_const_v<tValue> && std::is_copy_constructible_v<tValue>);
		struct xNode {
			union {
				uint32_t NextFreeIdIndex;
				uint32_t Key;
			};
			xHolder<tValue> ValueHolder;
		};
	public:
		X_INLINE bool Init(size_t Size)
		{
			assert(Size <= xIndexId::MaxIndexValue);
			assert(_IdPoolPtr == nullptr);
			assert(_NextFreeIdIndex == NoFreeIndex);
			assert(_InitedId == 0);

			_IdPoolPtr = new xNode[Size];
			_IdPoolSize = (size32_t)Size;
			_InitedId = 0;
			_NextFreeIdIndex = NoFreeIndex;

		    _Counter = xIndexId::TimeSeed();
			_Random32.seed(_Counter);
			return true;
		}

		X_INLINE std::enable_if_t<!RandomKey, bool> DebugInit(size_t Size, uint32_t InitCounter) {
			if(!Init(Size)) {
				return false;
			}
			_Counter = InitCounter;
			return true;
		}

		X_INLINE void Clean()
		{
			assert(_IdPoolPtr);
			for (size_t Index = 0 ; Index < _InitedId; ++Index) {
				auto & Node = _IdPoolPtr[Index];
				if (!(Node.Key & xIndexId::KeyInUseBitmask)) {
					continue;
				}
				Node.ValueHolder.Destroy();
			}
			delete [] Steal(_IdPoolPtr);
			_InitedId = 0;
			_IdPoolSize = 0;
			_NextFreeIdIndex = NoFreeIndex;
		}

		X_INLINE xIndexId Acquire() {
			uint32_t Index;
			xNode * NodePtr;
			if (_NextFreeIdIndex == NoFreeIndex) {
				if (X_UNLIKELY(_InitedId >= _IdPoolSize)) {
					return xIndexId::InvalidValue;
				}
				NodePtr = &_IdPoolPtr[Index = _InitedId++];
			} else {
				NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
			}
			uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
			Rand |= xIndexId::KeyInUseBitmask;
			Rand &= xIndexId::KeyMask;
			NodePtr->Key = Rand;
			try {
				NodePtr->ValueHolder.Create();
			} catch (...) {
				NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
				throw;
			}
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		X_INLINE xIndexId Acquire(const tValue & Value) {
			uint32_t Index;
			xNode * NodePtr;
			if (_NextFreeIdIndex == NoFreeIndex) {
				if (X_UNLIKELY(_InitedId >= _IdPoolSize)) {
					return xIndexId::InvalidValue;
				}
				NodePtr = &_IdPoolPtr[Index = _InitedId++];
			} else {
				NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
			}
			uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
			Rand |= xIndexId::KeyInUseBitmask;
			Rand &= xIndexId::KeyMask;
			NodePtr->Key = Rand;
			try {
				NodePtr->ValueHolder.CreateValue(Value);
			} catch (...) {
				NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
				throw;
			}
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		X_INLINE xIndexId Acquire(tValue && Value = {}) {
			uint32_t Index;
			xNode * NodePtr;
			if (_NextFreeIdIndex == NoFreeIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				NodePtr = &_IdPoolPtr[Index = _InitedId++];
			} else {
				NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
			}
			uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
			Rand |= xIndexId::KeyInUseBitmask;
			Rand &= xIndexId::KeyMask;
			NodePtr->Key = Rand ;
			try {
				NodePtr->ValueHolder.CreateValue(std::move(Value));
			} catch (...) {
				NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
				throw;
			}
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		X_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			auto & Node = _IdPoolPtr[Index];
			Node.NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
			Node.ValueHolder.Destroy();
		}

		X_INLINE bool Check(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			return X_LIKELY(xIndexId::IsSafeKey(Key)) && X_LIKELY(Key == Node.Key);
		}

		X_INLINE xOptional<xRef<tValue>> CheckAndGet(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return {};
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			if (!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
				return {};
			}
			return *Node.ValueHolder;
		}

		X_INLINE xOptional<xRef<const tValue>> CheckAndGet(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return {};
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			if(!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
				return {};
			}
			return *Node.ValueHolder;
		}

		X_INLINE xOptional<tValue> CheckAndRelease(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!X_LIKELY(Index < _IdPoolSize)) {
				return {};
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			if(!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
				return {};
			}
			auto DeferredRelese = xScopeGuard{[&](){
				Node.ValueHolder.Destroy();
			}};
			Node.NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
			return std::move(*Node.ValueHolder);
		}

		X_INLINE tValue & operator [] (const xIndexId& Id) {
			return *_IdPoolPtr[Id.GetIndex()].ValueHolder;
		}

		X_INLINE const tValue & operator [] (const xIndexId& Id) const {
			return *_IdPoolPtr[Id.GetIndex()].ValueHolder;
		}


	private:
		xNode *       _IdPoolPtr  = nullptr;
		size32_t      _IdPoolSize = 0;
		size32_t      _InitedId   = 0;
		uint32_t      _NextFreeIdIndex = NoFreeIndex;
		uint32_t      _Counter = 0;
		std::mt19937  _Random32;
		static constexpr const uint32_t NoFreeIndex = static_cast<uint32_t>(-1);
		static constexpr const uint32_t CounterStep = 1;
	};

}
