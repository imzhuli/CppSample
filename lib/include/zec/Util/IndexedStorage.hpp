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

		ZEC_API_STATIC_MEMBER uint_fast32_t TimeSeed();
		static constexpr uint32_t MaxIndexValue = static_cast<uint32_t>(0x7FFF'FFFFu);
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
			uint32_t Rand = (RandomKey ? _Random32() : ++_Counter) | xIndexId::KeyInUseBitmask;
			_IdPoolPtr[Index] = Rand;
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
		}

		ZEC_INLINE bool Check(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (Index >= _IdPoolSize) {
				return false;
			}
			auto Key = Id.GetKey();
			return (Key & xIndexId::KeyInUseBitmask) && (Key == _IdPoolPtr[Index]);
		}

		ZEC_INLINE bool CheckAndRelease(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			auto Key = Id.GetKey();
			if (Index >= _IdPoolSize || !(Key & xIndexId::KeyInUseBitmask) ||  Key != _IdPoolPtr[Index]) {
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
		static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);
	};

	template<typename tValue, bool RandomKey>
	class xIndexedStorage final
	: xNonCopyable
	{
		static_assert(!std::is_reference_v<tValue> && !std::is_const_v<tValue> && std::is_copy_constructible_v<tValue>);
	public:
		ZEC_INLINE bool Init(size_t Size)
		{
			assert(Size <= xIndexId::MaxIndexValue);
			assert(_IdPoolPtr == nullptr);
			assert(_NextFreeIdIndex == InvalidIndex);
			assert(_InitedId == 0);

			_IdPoolPtr = new uint32_t[Size];
			_StoragePtr = reinterpret_cast<tValue*>(new ubyte[Size * sizeof(tValue)]);
			_IdPoolSize = (size32_t)Size;
			_InitedId = 0;
			_NextFreeIdIndex = InvalidIndex;

		    _Counter = xIndexId::TimeSeed();
			_Random32.seed(_Counter);
			return true;
		}

		ZEC_INLINE void Clean()
		{
			assert(_IdPoolPtr);
			delete [] reinterpret_cast<ubyte*>(Steal(_StoragePtr));
			delete [] Steal(_IdPoolPtr);
			_InitedId = 0;
			_IdPoolSize = 0;
			_NextFreeIdIndex = InvalidIndex;
		}

		ZEC_INLINE xIndexId Acquire(const tValue & Value) {
			uint32_t Index;
			if (_NextFreeIdIndex == InvalidIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				Index = _InitedId++;
			} else {
				Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex]);
			}
			uint32_t Rand = (RandomKey ? _Random32() : ++_Counter) | xIndexId::KeyInUseBitmask;
			_IdPoolPtr[Index] = Rand ;
			new ((void*)(_StoragePtr + Index)) tValue{ Value };
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE xIndexId Acquire(tValue && Value = {}) {
			uint32_t Index;
			if (_NextFreeIdIndex == InvalidIndex) {
				if (_InitedId >= _IdPoolSize) {
					return xIndexId::InvalidValue;
				}
				Index = _InitedId++;
			} else {
				Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex]);
			}
			uint32_t Rand = (RandomKey ? _Random32() : ++_Counter) | xIndexId::KeyInUseBitmask;
			_IdPoolPtr[Index] = Rand ;
			new ((void*)(_StoragePtr + Index)) tValue{ std::move(Value) };
			return { (static_cast<uint64_t>(Rand) << 32) + Index };
		}

		ZEC_INLINE void Release(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
			_StoragePtr[Index].~tValue();
		}

		ZEC_INLINE bool Check(const xIndexId& Id) {
			uint32_t Index = Id.GetIndex();
			if (Index >= _IdPoolSize) {
				return false;
			}
			auto Key = Id.GetKey();
			return (Key & xIndexId::KeyInUseBitmask) && (Key == _IdPoolPtr[Index]);
		}

		ZEC_INLINE xOptional<xRef<const tValue>> CheckAndGet(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			auto Key = Id.GetKey();
			if (Index >= _IdPoolSize || !(Key & xIndexId::KeyInUseBitmask) || Key != _IdPoolPtr[Index]) {
				return {};
			}
			return _StoragePtr[Index];
		}

		ZEC_INLINE xOptional<tValue> CheckAndRelease(const xIndexId& Id) const {
			uint32_t Index = Id.GetIndex();
			auto Key = Id.GetKey();
			if (Index >= _IdPoolSize || !(Key & xIndexId::KeyInUseBitmask) || Key != _IdPoolPtr[Index]) {
				return {};
			}
			auto DeferedRelese = xScopeGuard{[&](){
				_StoragePtr[Index].~tValue();
			}};
			_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
			return std::move(_StoragePtr[Index]);
		}

		ZEC_INLINE const tValue & Get(const xIndexId& Id) const {
			return _StoragePtr[Id.GetIndex()];
		}

		template<typename tAssignValue>
		ZEC_INLINE void Set(const xIndexId& Id, tAssignValue && Value) {
			_StoragePtr[Id.GetIndex()] = std::forward<tAssignValue>(Value);
		}


	private:
		size32_t      _InitedId   = 0;
		size32_t      _IdPoolSize = 0;
		uint32_t*     _IdPoolPtr  = nullptr;
		uint32_t      _NextFreeIdIndex = InvalidIndex;
		tValue *      _StoragePtr = nullptr;
		uint32_t      _Counter = 0;
		std::mt19937  _Random32;
		static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);
	};

}
