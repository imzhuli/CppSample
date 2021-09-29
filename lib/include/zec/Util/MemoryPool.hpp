#pragma once

#include "../Common.hpp"
#include "../List.hpp"
#include "./Memory.hpp"
#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <new>

ZEC_NS
{
	struct xMemoryPoolOptions {
		xAllocator *  Allocator = &DefaultAllocator;
		size_t        InitSize = 64;
		size_t        Addend = 128;
		size_t        MultiplierBy100th = 0;
		size_t        MaxSizeIncrement = 1024;
		size_t        MaxPoolSize = std::numeric_limits<ssize_t>::max();
	};

	template<typename T>
	class xMemoryPool final
	{
		static_assert(!std::is_reference_v<T>);

		// !!! important note:
		// here we are using struct (not union), so that if the Constructor throws any exception and is caught outside,
		// the memory pool still remains usable.
		struct xTypeWrapper final {
			alignas(T) ubyte xObjectHolder[sizeof(T)];
			xTypeWrapper * pNext;

			ZEC_INLINE T* Construct() { auto pTarget = ((void*)&xObjectHolder); new (pTarget) T; return static_cast<T*>(pTarget); }
			template<typename ... CArgs>
			ZEC_INLINE T* ConstructWith(CArgs && ... cargs) { auto pTarget = ((void*)&xObjectHolder); new (pTarget) T{std::forward<CArgs>(cargs)...}; return static_cast<T*>(pTarget); }

			ZEC_INLINE void Destruct() noexcept { reinterpret_cast<T*>(&xObjectHolder)->~T(); }
		};

		struct xBlock : public xListNode {
			size_t Count;
			size_t InitCount = 0;
			xTypeWrapper ResourcePool[1];

			ZEC_INLINE xBlock(size_t count) noexcept : Count(count) {}
			xBlock(xBlock&&) = delete;
		};

	private:
		xAllocator *  hAlloc               = nullptr;
		size_t        cInitSize            = 0;
		size_t        cAddend              = 0;
		size_t        cMultiplierBy100th   = 0;
		size_t        cMaxSizeIncrement    = 1000;
		size_t        cMaxPoolSize = std::numeric_limits<ssize_t>::max();

		xList<xBlock>    _BlockList;
		xTypeWrapper *   _NextFreeNode = nullptr;
		size_t           _TotalSize = 0;

	public:
		ZEC_INLINE bool Init(const xMemoryPoolOptions & Options) {
			assert(Options.Allocator);
			assert(Options.MultiplierBy100th || Options.Addend);
			assert(Options.InitSize > 1 && Options.MaxSizeIncrement > 0 && Options.MaxPoolSize >= Options.InitSize);

			hAlloc = Options.Allocator;
			cInitSize = Options.InitSize;
			cAddend = Options.Addend;
			cMultiplierBy100th = Options.MultiplierBy100th;
			cMaxSizeIncrement = Options.MaxSizeIncrement;
			cMaxPoolSize = Options.MaxPoolSize;

			return AllocBlock(cInitSize) != nullptr;
		}

		ZEC_INLINE void Clean() {
			for(auto & block : _BlockList) {
				/**
				 * call to block.~xBlock() is omitted,
				 * since all blocks hold nothing usable, except the block_list_node subobject,
				 * which will be simply dropped in later call to _BlockList.release();
				 */
				hAlloc->Free(&block);
			}
			_BlockList.Release();
			_NextFreeNode = nullptr;
			_TotalSize = 0;
		}

		ZEC_INLINE T * Create() {
			if (_NextFreeNode) {
				T * pTarget = _NextFreeNode->Construct();
				_NextFreeNode = _NextFreeNode->pNext;
				return pTarget;
			}
			auto pLastBlock = _BlockList.Tail();
			if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
				return nullptr;
			}
			xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
			T * pTarget = pWrapper->Construct();
			++pLastBlock->InitCount;
			return pTarget;
		}

		template<typename... CArgs>
		ZEC_INLINE T * CreateValue(CArgs && ... cargs) {
			if (_NextFreeNode) {
				T * pTarget = _NextFreeNode->ConstructWith(std::forward<CArgs>(cargs)...);
				_NextFreeNode = _NextFreeNode->pNext;
				return pTarget;
			}
			auto pLastBlock = _BlockList.tail();
			if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
				return nullptr;
			}
			xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
			T * pTarget = pWrapper->ConstructWith(std::forward<CArgs>(cargs)...);
			++pLastBlock->InitCount;
			return pTarget;
		}

		ZEC_INLINE void Destroy(T * pTarget) {
			assert(pTarget);
			xTypeWrapper * pWrapper = reinterpret_cast<xTypeWrapper*>(
				reinterpret_cast<ubyte*>(pTarget) - offsetof(xTypeWrapper, xObjectHolder)
			);
			pWrapper->Destruct();
			pWrapper->pNext = _NextFreeNode;
			_NextFreeNode = pWrapper;
		}

	private:
		ZEC_INLINE xBlock * ExtendPool() {
			size_t maxAddSize = std::min(cMaxSizeIncrement, cMaxPoolSize - _TotalSize);
			if (!maxAddSize) {
				return nullptr;
			}
			size_t addSize = std::min(_TotalSize * cMultiplierBy100th / 100 + cAddend, maxAddSize);
			return AllocBlock(addSize);
		}

		xBlock * AllocBlock(size_t count) {
			assert(count);
			size_t totalSize = (sizeof(xBlock) - sizeof(xTypeWrapper)) + sizeof(xTypeWrapper) * count;
			xBlock * pBlock = (xBlock*)hAlloc->Alloc(totalSize, AllocAlignSize<xBlock>);
			if (!pBlock) {
				return nullptr;
			}
			new ((void*)pBlock) xBlock(count);
			// Init queue:
			auto & block = *pBlock;
			_BlockList.AddTail(block);
			_TotalSize += count;
			return pBlock;
		}

	};

}
