#include <zec/Util/Memory.hpp>
#include <atomic>
#include <new>

ZEC_NS
{
	xAllocator DefaultAllocator;

	void * xAllocator::Alloc(size_t vxSize, size_t vxAlignment)
	{
		assert(vxSize % vxAlignment == 0);
		auto p = ::ZecAlignedAlloc(vxSize, vxAlignment);
		if (!p) {
			throw std::bad_alloc();
		}
		return p;
	}

	void xAllocator::Free(const void * vpObject)
	{
		::ZecAlignedFree(const_cast<void*>(vpObject));
	}

	xAllocator::~xAllocator()
	{}

}
