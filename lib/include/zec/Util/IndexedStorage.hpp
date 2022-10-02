#pragma once
#include "../Common.hpp"
#include <random>

ZEC_NS
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
	 *      the highest bit in Index part and is always zero,
	 *      the highest bit in Key part is always one (aka: KeyInUseBitmask == 0x8000'0000u),
	 *      so the id pool could use one 32bit integer to store an index to next free node in chain
	 *      while the index itself is free, or a key value with KeyInUseBitmask set while the index is in use;
	 *    Especially, since all allocated index has KeyInUseBitmask set to 1, a valid index id is never zero;
	 * */
	class xIndexId final
	{
	public:
		ZEC_INLINE xIndexId() = default;
		ZEC_INLINE constexpr xIndexId(uint64_t Value) : _Value(Value) {};
		ZEC_INLINE constexpr operator uint64_t () const { return _Value; }

		ZEC_INLINE uint32_t GetIndex() const { return (uint32_t)_Value; }
		ZEC_INLINE uint32_t GetKey() const { return static_cast<uint32_t>(_Value >> 32);}

		static constexpr uint64_t InvalidValue  = static_cast<uint64_t>(0);

	private:
		uint64_t _Value;

		template<bool RandomKey>
		friend class xIndexIdPool;
		template<typename tValue, bool RandomKey>
		friend class xIndexedStorage;

		ZEC_API_STATIC_MEMBER uint32_t TimeSeed();
		static constexpr const uint32_t MaxIndexValue = static_cast<uint32_t>(0x7FFF'FFFFu);
		static constexpr const uint32_t KeyInUseBitmask = 0x8000'0000u;
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
			assert(_NextFreeIdIndex == InvalidIndex);
			assert(_InitedId == 0);

			_IdPoolPtr = new uint32_t[Size];
			_IdPoolSize = (size32_t)Size;
			_InitedId = 0;
			_NextFreeIdIndex = InvalidIndex;

		    _Counter = xIndexId::TimeSeed();
			_Random32.seed(_Counter);
			return true;
		}

		ZEC_INLINE std::enable_if_t<!RandomKey, bool> DebugInit(size_t Size, uint32_t InitCounter = 0) {
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
			_NextFreeIdIndex = InvalidIndex;
		}

		ZEC_INLINE xIndexId Acquire() {
			uint32_t Index;
			if (_NextFreeIdIndex == InvalidIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				Index = _InitedId++;
			} else {
				Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex]);
			}
			uint32_t Rand = (RandomKey ? _Random32() : (_Counter += CounterStep)) | xIndexId::KeyInUseBitmask;
			_IdPoolPtr[Index] = Rand;
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
		}

		ZEC_INLINE bool Check(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			return ZEC_LIKELY(Key & xIndexId::KeyInUseBitmask) && ZEC_LIKELY(Key == _IdPoolPtr[Index]);
		}

		ZEC_INLINE bool CheckAndRelease(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			if(!ZEC_LIKELY(Key & xIndexId::KeyInUseBitmask) || !ZEC_LIKELY(Key == _IdPoolPtr[Index])) {
				return false;
			}
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
			return true;
		}

	private:
		size32_t      _InitedId   = 0;
		size32_t      _IdPoolSize = 0;
		uint32_t*     _IdPoolPtr  = nullptr;
		uint32_t      _NextFreeIdIndex = InvalidIndex;
		uint32_t      _Counter = 0;
		std::mt19937  _Random32;
		static constexpr const uint32_t CounterStep = 1;
		static constexpr const uint32_t InvalidIndex = static_cast<uint32_t>(-1);
	};

	template<typename tValue, bool RandomKey>
	class xIndexedStorage final
	: xNonCopyable
	{
		static_assert(!std::is_reference_v<tValue> && !std::is_const_v<tValue> && std::is_copy_constructible_v<tValue>);
		struct xNode {
			union {
				uint32_t NextFreeIndex;
				uint32_t Key;
			};
			xHolder<tValue> ValueHolder;
		};
	public:
		ZEC_INLINE bool Init(size_t Size)
		{
			assert(Size <= xIndexId::MaxIndexValue);
			assert(_IdPoolPtr == nullptr);
			assert(_NextFreeIdIndex == InvalidIndex);
			assert(_InitedId == 0);

			_IdPoolPtr = new xNode[Size];
			_IdPoolSize = (size32_t)Size;
			_InitedId = 0;
			_NextFreeIdIndex = InvalidIndex;

		    _Counter = xIndexId::TimeSeed();
			_Random32.seed(_Counter);
			return true;
		}

		ZEC_INLINE std::enable_if_t<!RandomKey, bool> DebugInit(size_t Size, uint32_t InitCounter) {
			if(!Init(Size)) {
				return false;
			}
			_Counter = InitCounter;
			return true;
		}

		ZEC_INLINE void Clean()
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
			_NextFreeIdIndex = InvalidIndex;
		}

		ZEC_INLINE xIndexId Acquire(const tValue & Value) {
			uint32_t Index;
			xNode * NodePtr;
			if (_NextFreeIdIndex == InvalidIndex) {
				if (ZEC_UNLIKELY(_InitedId >= _IdPoolSize)) {
					return xIndexId::InvalidValue;
				}
				NodePtr = &_IdPoolPtr[Index = _InitedId++];
			} else {
				NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
			}
			uint32_t Rand = (RandomKey ? _Random32() : (_Counter += CounterStep)) | xIndexId::KeyInUseBitmask;
			NodePtr->Key = Rand;
			try {
				NodePtr->ValueHolder.CreateValue(Value);
			} catch (...) {
				NodePtr->Index = Steal(_NextFreeIdIndex, Index);
				throw;
			}
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE xIndexId Acquire(tValue && Value = {}) {
			uint32_t Index;
			xNode * NodePtr;
			if (_NextFreeIdIndex == InvalidIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				NodePtr = &_IdPoolPtr[Index = _InitedId++];
			} else {
				NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
			}
			uint32_t Rand = (RandomKey ? _Random32() : (_Counter += CounterStep)) | xIndexId::KeyInUseBitmask;
			NodePtr->Key = Rand ;
			try {
				NodePtr->ValueHolder.CreateValue(std::move(Value));
			} catch (...) {
				NodePtr->Index = Steal(_NextFreeIdIndex, Index);
				throw;
			}
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			auto & Node = _IdPoolPtr[Index];
			Node.NextFreeIndex = Steal(_NextFreeIdIndex, Index);
			Node.ValueHolder.Destroy();
		}

		ZEC_INLINE bool Check(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			return ZEC_LIKELY(Key & xIndexId::KeyInUseBitmask) && ZEC_LIKELY(Key == Node.Key);
		}

		ZEC_INLINE xOptional<xRef<tValue>> CheckAndGet(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return false;
			}
			auto Key = Id.GetKey();
			auto & Node = _IdPoolPtr[Index];
			if (!ZEC_LIKELY(Key & xIndexId::KeyInUseBitmask) || !ZEC_LIKELY(Key == Node.Key)) {
				return {};
			}
			return *Node.ValueHolder;
		}

		ZEC_INLINE xOptional<xRef<const tValue>> CheckAndGet(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return {};
			}
			auto Key = Id.GetKey();
			auto Node = _IdPoolPtr[Index];
			if(!ZEC_LIKELY(Key == Node.Key)) {
				return {};
			}
			return *Node.ValueHolder;
		}

		ZEC_INLINE xOptional<tValue> CheckAndRelease(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			if (!ZEC_LIKELY(Index < _IdPoolSize)) {
				return {};
			}
			auto Key = Id.GetKey();
			auto Node = _IdPoolPtr[Index];
			if(!ZEC_LIKELY(Key == Node.Key)) {
				return {};
			}
			auto DeferredRelese = xScopeGuard{[&](){
				Node.ValueHolder.Destroy();
			}};
			Node.NextFreeIndex = Steal(_NextFreeIdIndex, Index);
			return std::move(*Node.ValueHolder);
		}

		ZEC_INLINE tValue & Get(const xIndexId& Id) {
			return *_IdPoolPtr[Id.GetIndex()].ValueHolder;
		}

		ZEC_INLINE const tValue & Get(const xIndexId& Id) const {
			return *_IdPoolPtr[Id.GetIndex()].ValueHolder;
		}

		template<typename tAssignValue>
		ZEC_INLINE void Set(const xIndexId& Id, tAssignValue && Value) {
			*_IdPoolPtr[Id.GetIndex()].ValueHolder = std::forward<tAssignValue>(Value);
		}

	private:
		xNode *       _IdPoolPtr  = nullptr;
		size32_t      _IdPoolSize = 0;
		size32_t      _InitedId   = 0;
		uint32_t      _NextFreeIdIndex = InvalidIndex;
		uint32_t      _Counter = 0;
		std::mt19937  _Random32;
		static constexpr const uint32_t InvalidIndex = static_cast<uint32_t>(-1);
		static constexpr const uint32_t CounterStep = 1;
	};

}
