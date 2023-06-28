#pragma once
#include "./Common.hpp"
#include <type_traits>

X_NS
{
    // xList<tNode> type, with tNode extends xListNode
	template<typename tNode>
	class xQueue;

	class xQueueNode
	{
    private:
		template<typename tNode>
		friend class xQueue;
	private:
		xQueueNode * NextPtr = nullptr;
    public:
        X_INLINE xQueueNode() = default;
        X_INLINE xQueueNode(const xQueueNode &) {}
		X_INLINE xQueueNode & operator = (const xQueueNode & Other) noexcept { Pass(); return *this; }
        X_INLINE ~xQueueNode() { assert(!NextPtr); }
    };

	template<typename tNode>
	class xQueue final
    : xNonCopyable
	{
	private:
		static_assert(std::is_base_of_v<xQueueNode, tNode>);
		static_assert(!std::is_reference_v<tNode> && !std::is_const_v<tNode>);

    private:
        xQueueNode * FirstPtr = nullptr;
        xQueueNode * LastPtr  = nullptr;

        struct xForwardIterator
        {
        private:
            friend class xQueue;
            xQueueNode* NodePtr;

            xForwardIterator(xQueueNode * NodePtr = nullptr) : NodePtr(NodePtr) {}

        public:
            xForwardIterator(const xForwardIterator &) = default;
            xForwardIterator& operator=(const xForwardIterator &) = default;

            X_INLINE void operator ++ () {
                assert(NodePtr);
                NodePtr = NodePtr->NextPtr;
            };
            X_INLINE bool operator == (const xForwardIterator & Other) const {
                return NodePtr == Other.NodePtr;
            }
            X_INLINE bool operator != (const xForwardIterator & Other) const {
                return NodePtr != Other.NodePtr;
            }
            X_INLINE tNode & operator * () const {
                assert(NodePtr);
                return static_cast<tNode&>(*NodePtr);
            }
            X_INLINE tNode & operator -> () const {
                assert(NodePtr);
                return &static_cast<tNode&>(*NodePtr);
            }
        };

    public:
        X_INLINE void Push(tNode & Node) {
            auto & QueueNode = static_cast<xQueueNode &>(Node);
            assert(!QueueNode.NextPtr);
            if (!LastPtr) {
                assert(!FirstPtr);
                FirstPtr = LastPtr = &QueueNode;
                return;
            } else {
                assert(!LastPtr->NextPtr);
                LastPtr->NextPtr = &QueueNode;
                LastPtr = &QueueNode;
            }
        }
        X_INLINE tNode * Peek() {
            return static_cast<tNode*>(FirstPtr);
        }
        X_INLINE void RemoveFront() {
            assert(FirstPtr);
            if (!(FirstPtr = Steal(FirstPtr->NextPtr))) {
                LastPtr = nullptr;
            }
        }
        X_INLINE tNode * Pop() {
            if (auto NodePtr = Peek()) {
                RemoveFront();
                return NodePtr;
            }
            return nullptr;
        }

        X_INLINE xForwardIterator begin() {
            return xForwardIterator(FirstPtr);
        }

        X_INLINE xForwardIterator end() {
            return xForwardIterator();
        }

    };

}
