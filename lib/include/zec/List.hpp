#pragma once
#include "./Common.hpp"
#include <type_traits>

ZEC_NS
{
	// xList<tNode> type, with tNode extends xListNode
	template<typename tNode>
	class xList;

	class xListNode
	{
	private:
		xListNode* pPrev;
		xListNode* pNext;

		template<typename tNode>
		friend class xList;

	protected:
		ZEC_INLINE xListNode() noexcept { Reset(); }
		ZEC_INLINE ~xListNode() noexcept { DetachUnsafe(); }
		xListNode(xListNode &&) = delete;

		ZEC_INLINE void Reset() {
			pPrev = pNext = this;
		}
		ZEC_INLINE void Detach() {
			DetachUnsafe();
			Reset();
		}
		ZEC_INLINE void TakePlaceOf(xListNode& other) {
			TakePlaceOfUnsafe(other);
			other.Reset();
		}
		ZEC_INLINE bool Linked() const {
			return pPrev != this;
		}

	private:
		ZEC_INLINE void AppendTo(xListNode& prev_node) {
			xListNode& next_node = *prev_node.pNext;
			prev_node.pNext = this;
			next_node.pPrev = this;
			pPrev = &prev_node;
			pNext = &next_node;
		}

		ZEC_INLINE void DetachUnsafe() {
			pPrev->pNext = pNext;
			pNext->pPrev = pPrev;
		}

		ZEC_INLINE void TakePlaceOfUnsafe(xListNode& other) {
			pPrev = other.pPrev;
			pNext = other.pNext;
			pNext->pPrev = this;
			pPrev->pNext = this;
		}

	};

	template<typename tNode>
	class xList
	{
	private:
		static_assert(std::is_base_of_v<xListNode, tNode>);
		static_assert(!std::is_reference_v<tNode> && !std::is_const_v<tNode>);
		xListNode _Head;

	public:
		xList() = default;
		xList(const xList&) = delete;
		ZEC_INLINE xList(xList&& other) {
			GrabListTail(other);
		}
		ZEC_INLINE ~xList() {
			assert(IsEmpty());
		}

	private:
		template<bool isConst>
		class xForwardIteratorTemplate
		{
			using xBaseNode   = std::conditional_t<isConst, const xListNode, xListNode>;
			using xExtendNode = std::conditional_t<isConst, const tNode, tNode>;
		private:
			xBaseNode* pTarget;
			xBaseNode* pNext;

		private:
			ZEC_INLINE xExtendNode* Ptr() const { return static_cast<xExtendNode*>(pTarget); }
			ZEC_INLINE void Copy(xBaseNode* n) { pTarget = n; pNext = n->pNext; }

		public:
			// construct:
			ZEC_INLINE xForwardIteratorTemplate() = delete;
			ZEC_INLINE xForwardIteratorTemplate(xBaseNode* n) { Copy(n); }
			// for use of xList::end(),
			ZEC_INLINE xForwardIteratorTemplate(xBaseNode* n, const std::nullptr_t &) { pTarget = n, pNext = nullptr; }

			// Copy:
			ZEC_INLINE xForwardIteratorTemplate(const xForwardIteratorTemplate& it) = default;
			ZEC_INLINE xForwardIteratorTemplate& operator=(const xForwardIteratorTemplate& it) = default;

			// cast:
			ZEC_INLINE xExtendNode* operator->() const { return Ptr(); }
			ZEC_INLINE xExtendNode& operator*() const { return *Ptr(); }

			// compare:
			ZEC_INLINE bool operator==(const xForwardIteratorTemplate& it) const { return pTarget == it.pTarget; }
			ZEC_INLINE bool operator!=(const xForwardIteratorTemplate& it) const { return pTarget != it.pTarget; }

			// traversing:
			ZEC_INLINE xForwardIteratorTemplate operator++() {
				Copy(pNext);
				return *this;
			}
			ZEC_INLINE xForwardIteratorTemplate operator++(int) {
				xForwardIteratorTemplate ret(*this);
				Copy(pNext);
				return ret;
			}
		};

	public:
		using xForwardIterator = xForwardIteratorTemplate<false>;
		using xForwardConstIterator = xForwardIteratorTemplate<true>;

	public:
		ZEC_INLINE bool IsEmpty() { return _Head.pNext == &_Head;  }
		ZEC_INLINE void AddHead(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).AppendTo(_Head);
		}
		ZEC_INLINE void AddTail(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).AppendTo(*_Head.pPrev);
		}
		ZEC_INLINE void GrabHead(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).DetachUnsafe();
			AddHead(rTarget);
		}
		ZEC_INLINE void GrabTail(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).DetachUnsafe();
			AddTail(rTarget);
		}
		ZEC_INLINE void GrabListHead(xList& other) {
			if (other.IsEmpty()) {
				return;
			};
			xListNode* remoteHead = other._Head.pNext;
			xListNode* remoteTail = other._Head.pPrev;
			other.Reset();

			xListNode* localHead = _Head.pNext;
			_Head.pNext = remoteHead;
			remoteHead->pPrev = &_Head;
			localHead->pPrev = remoteTail;
			remoteTail->pNext = localHead;
		}
		ZEC_INLINE void GrabListTail(xList& other) {
			if (other.IsEmpty()) {
				return;
			};
			xListNode* remoteHead = other._Head.pNext;
			xListNode* remoteTail = other._Head.pPrev;
			other._Head.Reset();

			xListNode* localTail = _Head.pPrev;
			_Head.pPrev = remoteTail;
			remoteTail->pNext = &_Head;
			localTail->pNext = remoteHead;
			remoteHead->pPrev = localTail;
		}
		ZEC_INLINE tNode * Head() {
			if (IsEmpty()) {
				return nullptr;
			}
			return static_cast<tNode*>(_Head.pNext);
		}
		ZEC_INLINE tNode * Tail() {
			if (IsEmpty()) {
				return nullptr;
			}
			return static_cast<tNode*>(_Head.pPrev);
		}
		ZEC_INLINE tNode * PopHead() {
			if (IsEmpty()) {
				return nullptr;
			}
			auto ret = _Head.pNext;
			ret->Detach();
			return static_cast<tNode*>(ret);
		}
		ZEC_INLINE tNode * PopTail() {
			if (IsEmpty()) {
				return nullptr;
			}
			auto ret = _Head.pPrev;
			ret->Detach();
			return static_cast<tNode*>(ret);
		}

		ZEC_INLINE xForwardIterator begin() { return xForwardIterator(_Head.pNext); }
		ZEC_INLINE xForwardIterator end() { return xForwardIterator(&_Head, nullptr); }

		ZEC_INLINE xForwardConstIterator begin() const { return xForwardConstIterator(_Head.pNext); }
		ZEC_INLINE xForwardConstIterator end() const { return xForwardConstIterator(&_Head, nullptr); }

		ZEC_INLINE void Release() { _Head.Reset(); }
	};

}
