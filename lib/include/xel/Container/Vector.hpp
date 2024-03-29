#pragma once
#include "../Common.hpp"
#include "../Memory.hpp"
#include "../View.hpp"
#include <cstring>
#include <new>

X_NS
{

	template<typename T>
	class xVector final
	{
		template<typename U>
		friend class xVector;

	public:
		X_INLINE xVector(xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc)
		, _Data(nullptr)
		, _Capacity(0)
		, _Size(0)
		{}

		X_INLINE xVector(xCapacityInit capacity, xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc)
		, _Capacity(capacity.value)
		, _Size(0)
		{
			assert(capacity.value > 0);
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);
		}

		X_INLINE xVector(size_t initSize, xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc), _Size(initSize)
		{
			_Capacity = (_Size + 15) & ~0x0F;
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);

			if constexpr(!std::is_trivial_v<T>) {
				for(auto & it : View()) {
					new ((void*)&it) T;
				}
			}
		}

		X_INLINE xVector(size_t initSize, const xZeroInit &, xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc), _Size(initSize)
		{
			_Capacity = (_Size + 15) & ~0x0F;
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);

			if constexpr(!std::is_trivial_v<T>) {
				for(auto & it : View()) {
					new ((void*)&it) T{};
				}
			}
			else {
				memset(Begin() + _Size, 0, sizeof(T) * _Size);
			}
		}

		X_INLINE xVector(size_t initSize, const T & vInitValue, xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc), _Size(initSize)
		{
			_Capacity = (_Size + 15) & ~0x0F;
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);
			for(auto & it : View()) {
				new ((void*)&it) T{vInitValue};
			}
		}

		template<typename U>
		X_INLINE xVector(xRangeView<U> range, xAllocator * alloc = &DefaultAllocator)
		: _Alloc(alloc), _Size(range.size())
		{
			_Capacity = (_Size + 15) & ~0x0F;
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);

			if constexpr(!std::is_trivial_v<T> || !std::is_same_v<T, U>) {
				auto dst = _Data;
				for(auto & it : range) {
					new ((void*)(dst++)) T{it};
				}
			}
			else {
				memcpy(_Data, range.begin(), sizeof(T) * range.size());
			}
		}

		template<typename U>
		X_INLINE xVector(const xVector<U> & other)
		: _Alloc(other._Alloc), _Capacity(other._Capacity), _Size(other._Size)
		{
			_Data = (T*)_Alloc->Alloc(sizeof(T) * _Capacity, AllocAlignSize<T>);
			assert(_Data);
			if constexpr(!std::is_trivial_v<T>) {
				auto dst = _Data;
				for(auto & it : other.View()) {
					new ((void*)(dst++)) T{it};
				}
			}
			else {
				memcpy(Begin(), other.Begin(), sizeof(T) * _Size);
			}
		}

		X_INLINE xVector(xVector && other)
		: _Alloc(other._Alloc), _Data(other._Data), _Capacity(other._Capacity), _Size(other._Size)
		{
			other._Data = nullptr;
			other._Capacity = other._Size = 0;
		}

		X_INLINE ~xVector()
		{
			if (_Data) {
				Clear();
				_Alloc->Free(_Data);
			}
		}

		X_INLINE T *                    Data()                { return _Data; }
		X_INLINE T *                    Data() const          { return _Data; }
		X_INLINE size_t                 Capacity() const      { return _Capacity; }
		X_INLINE size_t                 Size() const          { return _Size; }
		X_INLINE bool                   Empty() const         { return !_Size; }
		X_INLINE void                   Clear()               {
			if constexpr(!std::is_trivial_v<T>) {
				for(auto & elm : View()) {
					elm.~T();
				}
			}
			_Size = 0;
		}

		X_INLINE T *                    Begin()               { return _Data; }
		X_INLINE T *                    End()                 { return _Data + _Size; }
		X_INLINE xRangeView<T>          View()                { return xRangeView(_Data, _Size); }

		X_INLINE const T *              Begin() const         { return _Data; }
		X_INLINE const T *              End()   const         { return _Data + _Size; }
		X_INLINE xRangeView<const T>    View()  const         { return xRangeView((const T*)_Data, _Size); }

		X_INLINE T &                    operator[] (ptrdiff_t offset) { return *(Begin() + offset); }
		X_INLINE const T &              operator[] (ptrdiff_t offset) const { return *(Begin() + offset); }

		X_INLINE void Reserve(size_t vNewCap) { if (vNewCap <= _Capacity) { return; } ExpandCapacity(vNewCap); }

		X_INLINE void Resize(size_t vNewSize)
		{
			if (vNewSize <= _Size) {
				for(auto & it : xRangeView(Begin() + vNewSize, End())) {
					it.~T();
				}
				_Size = vNewSize;
				return;
			}

			if (vNewSize > _Capacity) {
				ExpandCapacity((vNewSize + 15) & ~0x0F);
			}

			if constexpr(!std::is_trivial_v<T>) {
				for(auto & it : xRangeView(End(), vNewSize - _Size)) {
					new ((void*)&it) T;
				}
			}
			_Size = vNewSize;
		}

		X_INLINE void Resize(size_t vNewSize, const xZeroInit &)
		{
			if (vNewSize <= _Size) {
				for(auto & it : xRangeView(Begin() + vNewSize, End())) {
					it.~T();
				}
				_Size = vNewSize;
				return;
			}

			if (vNewSize > _Capacity) {
				ExpandCapacity((vNewSize + 15) & ~0x0F);
			}

			if constexpr(!std::is_trivial_v<T>) {
				for(auto & it : xRangeView(End(), vNewSize - _Size)) {
					new ((void*)&it) T{};
				}
			}
			else {
				memset(Begin() + _Size, 0, sizeof(T) * (vNewSize - _Size));
			}
			_Size = vNewSize;
		}

		X_INLINE void Resize(size_t vNewSize, const T & vInitValue)
		{
			if (vNewSize <= _Size) {
				for(auto & it : xRangeView(Begin() + vNewSize, End())) {
					it.~T();
				}
				_Size = vNewSize;
				return;
			}

			if (vNewSize > _Capacity) {
				ExpandCapacity((vNewSize + 15) & ~0x0F);
			}

			for(auto & it : xRangeView(End(), vNewSize - _Size)) {
				new ((void*)&it) T{vInitValue};
			}
			_Size = vNewSize;
		}

		X_INLINE void CopyAppend(const T & obj)
		{
			CopyAppend(xRangeView(&obj, 1));
		}

		template<typename U>
		X_INLINE void CopyAppend(const U & u)
		{
			CopyAppend(xRangeView(&u, 1));
		}

		template<typename U>
		X_INLINE void CopyAppend(xRangeView<U> range)
		{
			size_t vNewSize = range.size() + _Size;
			if (vNewSize > _Capacity) {
				ExpandCapacity((vNewSize + 15) & ~0x0F);
			}
			if constexpr(!std::is_trivial_v<T> || !std::is_same_v<T, U>) {
				auto src = range.begin();
				for(auto & elm : xRangeView(End(), vNewSize - _Size)) {
					new ((void*)&elm) T{*(src++)};
				}
			}
			else {
				memcpy(End(), range.begin(), sizeof(T) * range.size());
			}
			_Size = vNewSize;
		}

		template<typename U, size_t N>
		X_INLINE void CopyAppend(U (&arr)[N])
		{
			CopyAppend(xRangeView{arr});
		}

		template<typename U>
		X_INLINE void MoveAppend(U && u)
		{
			MoveAppend(xRangeView(&u, 1));
		}

		X_INLINE void MoveAppend(xRangeView<T> range)
		{
			size_t vNewSize = range.size() + _Size;
			if (vNewSize > _Capacity) {
				ExpandCapacity((vNewSize + 15) & ~0x0F);
			}
			if constexpr(!std::is_trivial_v<T>) {
				auto src = range.begin();
				for(auto & it : xRangeView(End(), vNewSize - _Size)) {
					new ((void*)&it) T{std::move(*(src++))};
				}
			}
			else {
				memcpy(End(), range.begin(), sizeof(T) * range.size());
			}
			_Size = vNewSize;
		}

		X_INLINE void Erase(T * iter)
		{
			Erase(xRangeView(iter, 1));
		}

		X_INLINE void Erase(xRangeView<T> range)
		{
			assert(Begin() <= range.begin());
			assert(range.end() <= End());
			if constexpr(!std::is_trivial_v<T>) {
				for (auto & elm : range) {
					elm.~T();
				}
				auto dst = range.begin();
				auto src = range.end();
				auto end = End();
				while (src != end) {
					*(dst++) = std::move(*(src++));
				}
			}
			else {
				memmove(range.begin(), range.end(), sizeof(T) * (End() - range.end()));
			}
			_Size -= range.size();
		}

		X_INLINE void EraseByValue(const T & value)
		{
			auto it = Begin();
			auto end = End();
			while(it != end) {
				if (*it == value) {
					Erase(it);
					return;
				}
				++it;
			}
		}

		template<typename U>
		X_INLINE void Assign(xRangeView<U> range)
		{
			Clear();
			CopyAppend(range);
		}

	protected:
		// resize the underlying space for new size.
		// size element is not updated, and no initialization is performed!!!
		X_INLINE void ExpandCapacity(size_t cap)
		{
			assert(_Capacity < cap);
			auto data = (T*)_Alloc->Alloc(sizeof(T) * cap, AllocAlignSize<T>);
			assert(data);

			// move and clear old data
			if (_Data) {
				if constexpr(!std::is_trivial_v<T>) {
					auto dst = data;
					for(auto & elm : xRangeView(_Data, _Size)) {
						new ((void*)dst) T(std::move(elm));
						++dst;
					}
					for(auto & elm : xRangeView(_Data, _Size)) {
						elm.~T();
					}
				}
				else {
					memcpy(data, _Data, sizeof(T) * _Size);
				}
				_Alloc->Free(_Data);
			}

			_Data     = data;
			_Capacity = cap;
		}

	private:
		xAllocator *   _Alloc;
		T *            _Data;
		size_t         _Capacity;
		size_t         _Size;
	};

}