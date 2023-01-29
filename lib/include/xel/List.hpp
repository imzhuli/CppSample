#pragma once
#include "./Common.hpp"
#include <type_traits>

X_NS
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
		X_INLINE xListNode() noexcept { Reset(); }
		X_INLINE xListNode(const xListNode & Other) noexcept { Reset(); }
		X_INLINE ~xListNode() noexcept { DetachUnsafe(); }

		X_INLINE void Reset() {
			pPrev = pNext = this;
		}
		X_INLINE void Detach() {
			DetachUnsafe();
			Reset();
		}
		X_INLINE void TakePlaceOf(xListNode& other) {
			TakePlaceOfUnsafe(other);
			other.Reset();
		}
		X_INLINE bool Linked() const {
			return pPrev != this;
		}
		X_INLINE bool IsSafelyDetached() const {
			return pPrev == this && pNext == this;
		}

	private:
		X_INLINE void AppendTo(xListNode& prev_node) {
			xListNode& next_node = *prev_node.pNext;
			prev_node.pNext = this;
			next_node.pPrev = this;
			pPrev = &prev_node;
			pNext = &next_node;
		}

		X_INLINE void InsertBefore(xListNode& next_node) {
			AppendTo(*next_node.pPrev);
		}

		X_INLINE void DetachUnsafe() {
			pPrev->pNext = pNext;
			pNext->pPrev = pPrev;
		}

		X_INLINE void TakePlaceOfUnsafe(xListNode& other) {
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
		X_INLINE xList(xList&& other) {
			GrabListTail(other);
		}
		X_INLINE ~xList() {
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
			X_INLINE xExtendNode* Ptr() const { return static_cast<xExtendNode*>(pTarget); }
			X_INLINE void Copy(xBaseNode* n) { pTarget = n; pNext = n->pNext; }

		public:
			// construct:
			X_INLINE xForwardIteratorTemplate() = delete;
			X_INLINE xForwardIteratorTemplate(xBaseNode* n) { Copy(n); }
			// for use of xList::end(),
			X_INLINE xForwardIteratorTemplate(xBaseNode* n, const std::nullptr_t &) { pTarget = n, pNext = nullptr; }

			// Copy:
			X_INLINE xForwardIteratorTemplate(const xForwardIteratorTemplate& it) = default;
			X_INLINE xForwardIteratorTemplate& operator=(const xForwardIteratorTemplate& it) = default;

			// cast:
			X_INLINE xExtendNode* operator->() const { return Ptr(); }
			X_INLINE xExtendNode& operator*() const { return *Ptr(); }

			// compare:
			X_INLINE bool operator==(const xForwardIteratorTemplate& it) const { return pTarget == it.pTarget; }
			X_INLINE bool operator!=(const xForwardIteratorTemplate& it) const { return pTarget != it.pTarget; }

			// traversing:
			X_INLINE xForwardIteratorTemplate operator++() {
				Copy(pNext);
				return *this;
			}
			X_INLINE xForwardIteratorTemplate operator++(int) {
				xForwardIteratorTemplate ret(*this);
				Copy(pNext);
				return ret;
			}
		};

	public:
		using xForwardIterator = xForwardIteratorTemplate<false>;
		using xForwardConstIterator = xForwardIteratorTemplate<true>;

	public:
		X_INLINE bool IsEmpty() const { return _Head.pNext == &_Head;  }
		X_INLINE void AddHead(tNode& rTarget) {
			assert(static_cast<xListNode&>(rTarget).IsSafelyDetached());
			static_cast<xListNode&>(rTarget).AppendTo(_Head);
		}
		X_INLINE void AddTail(tNode& rTarget) {
			assert(static_cast<xListNode&>(rTarget).IsSafelyDetached());
			static_cast<xListNode&>(rTarget).AppendTo(*_Head.pPrev);
		}
		X_INLINE void GrabHead(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).DetachUnsafe();
			AddHead(rTarget);
		}
		X_INLINE void GrabTail(tNode& rTarget) {
			static_cast<xListNode&>(rTarget).DetachUnsafe();
			AddTail(rTarget);
		}
		X_INLINE void GrabListHead(xList& other) {
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
		X_INLINE void GrabListTail(xList& other) {
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
		X_INLINE tNode * Head() {
			if (IsEmpty()) {
				return nullptr;
			}
			return static_cast<tNode*>(_Head.pNext);
		}
		X_INLINE tNode * Tail() {
			if (IsEmpty()) {
				return nullptr;
			}
			return static_cast<tNode*>(_Head.pPrev);
		}
		X_INLINE tNode * PopHead() {
			if (IsEmpty()) {
				return nullptr;
			}
			auto ret = _Head.pNext;
			ret->Detach();
			return static_cast<tNode*>(ret);
		}
		X_INLINE tNode * PopTail() {
			if (IsEmpty()) {
				return nullptr;
			}
			auto ret = _Head.pPrev;
			ret->Detach();
			return static_cast<tNode*>(ret);
		}
		X_STATIC_INLINE void Remove(tNode& Node) {
			Node.Detach();
		}

		X_INLINE xForwardIterator begin() { return xForwardIterator(_Head.pNext); }
		X_INLINE xForwardIterator end() { return xForwardIterator(&_Head, nullptr); }

		X_INLINE xForwardConstIterator begin() const { return xForwardConstIterator(_Head.pNext); }
		X_INLINE xForwardConstIterator end() const { return xForwardConstIterator(&_Head, nullptr); }

		X_INLINE void ReleaseUnsafe() { _Head.Reset(); }
	};

}
