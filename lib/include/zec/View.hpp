#pragma once
#include "./Common.hpp"

ZEC_NS
{

	class xDataView final
	{
	private:
		using xIterator = const ubyte *;

	public:
		ZEC_INLINE xDataView()                                = default;
		ZEC_INLINE xDataView(const xDataView &)                = default;
		ZEC_INLINE xDataView(const void *pv, size_t sz)       : pData(pv), Size(sz) {}

		template<typename T, std::enable_if_t<!std::is_same_v<T, xDataView>, size_t> N>
		ZEC_INLINE xDataView(const T (&refArray)[N])
		: xDataView(&refArray, sizeof(refArray))
		{}

		template<size_t N>
		ZEC_INLINE explicit xDataView(const xDataView (&refArray)[N])
		: xDataView(&refArray, sizeof(refArray))
		{}

		ZEC_INLINE const void *        operator()() const    { return pData; }
		ZEC_INLINE operator const      ubyte* () const       { return static_cast<const ubyte *>(pData); }

		template<typename ITEM_TYPE>
		ZEC_INLINE const ITEM_TYPE *   data() const          { static_assert(!std::is_reference_v<ITEM_TYPE>); return static_cast<const ITEM_TYPE *>(pData); }
		ZEC_INLINE const ubyte *       data() const          { return static_cast<const ubyte*>(pData); }

		template<typename SIZE_TYPE>
		ZEC_INLINE SIZE_TYPE           size() const          { return static_cast<SIZE_TYPE>(Size); }
		ZEC_INLINE size_t              size() const          { return Size; }

		ZEC_INLINE xIterator           begin() const         { return data(); }
		ZEC_INLINE xIterator           end() const           { return data() + Size; }

	private:
		const void *    pData = nullptr;
		size_t          Size = 0;
	};

	template<typename T>
	class xRangeView final
	{
		static_assert(!std::is_reference_v<T>);

	public:
		ZEC_INLINE xRangeView() = default;

		ZEC_INLINE xRangeView(T* start, T* end)
		: _Start(start), _End(end), _Size(end - start)
		{
			assert(_Start <= _End);
		}

		template<typename N, typename = std::enable_if_t<std::is_integral_v<N> && !std::is_pointer_v<N>>>
		ZEC_INLINE xRangeView(T* start, N number)
		: _Start(start), _End(start + number), _Size(number)
		{
			assert(_Start <= _End);
		}

		template<size_t N>
		xRangeView(T (&start)[N])
		: _Start(start), _End(start + N), _Size(N)
		{}

		ZEC_INLINE T&        operator[] (ptrdiff_t off) const { return *(_Start + off); }
		ZEC_INLINE T*        begin() const { return _Start; }
		ZEC_INLINE T*        end()   const { return _End; }
		ZEC_INLINE size_t    size()  const { return _Size; }

	private:
		T *       _Start {};
		T *       _End   {};
		size_t    _Size  {};
	};

	template<typename T>
	class xSliceView final
	{
		static_assert(!std::is_reference_v<T>);
		using IteratorMemory = std::conditional_t<std::is_const_v<T>, const ubyte *, ubyte*>;
		using IteratorInitPointer = std::conditional_t<std::is_const_v<T>, const void *, void*>;

	public:
		class xIterator
		{
		public:
			ZEC_INLINE xIterator(IteratorInitPointer ptr, size_t stride)
			: _Memory(reinterpret_cast<IteratorMemory>(ptr)), _Stride(stride)
			{}

			ZEC_INLINE xIterator(const xIterator &) = default;
			ZEC_INLINE ~xIterator() = default;

			ZEC_INLINE T * operator ->() const { return reinterpret_cast<T*>(_Memory); }
			ZEC_INLINE T & operator *() const { return *reinterpret_cast<T*>(_Memory); }

			ZEC_INLINE xIterator & operator ++() { _Memory += _Stride; return *this; }
			ZEC_INLINE bool operator == (const xIterator & other) { return _Memory == other._Memory; }
			ZEC_INLINE bool operator != (const xIterator & other) { return _Memory != other._Memory; }

		private:
			IteratorMemory    _Memory;
			size_t            _Stride;
		};

	public:
		ZEC_INLINE xSliceView() = default;

		ZEC_INLINE xSliceView(T* start, size_t number, size_t stride = sizeof(T))
		: _Start(reinterpret_cast<IteratorMemory>(start))
		, _End(_Start + number * stride)
		, _Stride(stride)
		, _Size(number)
		{}

		ZEC_INLINE xIterator  begin() const { return xIterator(_Start, _Stride); }
		ZEC_INLINE xIterator  end()   const { return xIterator(_End, 0); }

		ZEC_INLINE T&        operator[] (ptrdiff_t off) const { return *reinterpret_cast<T*>(_Start + off * _Stride); }
		ZEC_INLINE size_t    stride() const { return _Stride; }
		ZEC_INLINE size_t    size()  const { return _Size; }

	private:
		IteratorMemory     _Start    { nullptr };
		IteratorMemory     _End      { nullptr };
		size_t             _Stride   { sizeof(T) };
		size_t             _Size     { 0 };
	};

}
