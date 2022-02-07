#pragma once
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XEL_RBNODE_RED          ((uint32_t)(0x01))
#define XEL_RBNODE_COLORMASK    ((uint32_t)(0x01))

/* Node */
typedef struct XelRBNode XelRBNode;
struct XelRBNode {
    XelRBNode *   ParentPtr;
    XelRBNode *   LeftNodePtr;
    XelRBNode *   RightNodePtr;
    uint32_t      Flags;
};

ZEC_API void XRBN_Link(XelRBNode *Parent, XelRBNode **l, XelRBNode *n);
ZEC_API void XRBN_UnlinkStale(XelRBNode * NodePtr);

static inline void XRBN_Init(XelRBNode * NodePtr) {
    XelRBNode InitValue = {};
    *NodePtr = InitValue;
}

static inline bool XRBN_IsRoot(XelRBNode * NodePtr) {
    return !NodePtr->ParentPtr;
}
static inline bool XRBN_IsLeaf(XelRBNode * NodePtr) {
    return !NodePtr->LeftNodePtr && !NodePtr->RightNodePtr;
}
static inline bool XRBN_IsRed(XelRBNode * NodePtr) {
    return NodePtr->Flags & XEL_RBNODE_RED;
}
static inline bool XRBN_IsGenericRed(XelRBNode * NodePtr) {
    return NodePtr && (NodePtr->Flags & XEL_RBNODE_RED);
}
static inline bool XRBN_IsBlack(XelRBNode * NodePtr) {
    return !XRBN_IsRed(NodePtr);
}
static inline bool XRBN_IsGenericBlack(XelRBNode * NodePtr) {
    return !NodePtr || !XRBN_IsRed(NodePtr);
}
static inline void XRBN_MarkRed(XelRBNode * NodePtr) {
    NodePtr->Flags |= XEL_RBNODE_RED;
}
static inline void XRBN_MarkBlack(XelRBNode * NodePtr) {
    NodePtr->Flags &= ~XEL_RBNODE_RED;
}

static inline void* XRBN_Cast(XelRBNode* NodePtr, size_t NodeMemberOffset) {
    if (!NodePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)NodePtr - NodeMemberOffset);
}

static inline void XRBN_Unlink(XelRBNode * NodePtr) {
    XRBN_UnlinkStale(NodePtr);
    XRBN_Init(NodePtr);
}

static inline XelRBNode * XRBN_LeftMost(XelRBNode * NodePtr) {
    assert(NodePtr);
    while (NodePtr->LeftNodePtr) {
        NodePtr = NodePtr->LeftNodePtr;
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_RightMost(XelRBNode * NodePtr) {
    assert(NodePtr);
    while (NodePtr->RightNodePtr) {
        NodePtr = NodePtr->RightNodePtr;
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_LeftDeepest(XelRBNode * NodePtr) {
    assert(NodePtr);
    while(true) {
        if (NodePtr->LeftNodePtr) {
            NodePtr = NodePtr->LeftNodePtr;
        }
        else if (NodePtr) {
            NodePtr = NodePtr->RightNodePtr;
        }
        else {
            break;
        }
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_RightDeepest(XelRBNode * NodePtr) {
    assert(NodePtr);
    while(true) {
        if (NodePtr) {
            NodePtr = NodePtr->RightNodePtr;
        }
        else if (NodePtr->LeftNodePtr) {
            NodePtr = NodePtr->LeftNodePtr;
        }
        else {
            break;
        }
    }
    return NodePtr;
}

static inline XelRBNode * XRBN_Prev(XelRBNode * NodePtr) {
    XelRBNode * ParentPtr;
    if (NodePtr->LeftNodePtr) {
        return XRBN_RightMost(NodePtr->LeftNodePtr);
    }
    while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->LeftNodePtr)) {
        NodePtr = ParentPtr;
    }
    return ParentPtr;
}

static inline XelRBNode * XRBN_Next(XelRBNode * NodePtr) {
    XelRBNode * ParentPtr;
    if (NodePtr->RightNodePtr) {
        return XRBN_LeftMost(NodePtr->RightNodePtr);
    }
    while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->RightNodePtr)) {
        NodePtr = ParentPtr;
    }
    return ParentPtr;
}

#define XRBN_ENTRY(_What, Type, Member) ((Type*)(XRBN_Cast((_What), offsetof(Type, Member))))

/* Tree */
typedef struct XelRBTree XelRBTree;
struct XelRBTree {
    XelRBNode * RootPtr;
};

typedef int XRBT_KeyCompare(XelRBTree * TreePtr, const void * KeyPtr, XelRBNode * NodePtr);

static inline void XRBT_Init(XelRBTree* TreePtr) {
    XelRBTree InitValue = {};
    *TreePtr = InitValue;
}

static inline bool XRBT_IsEmpty(XelRBTree* TreePtr) {
    return !TreePtr->RootPtr;
}

static inline void* XRBT_Cast(XelRBTree* TreePtr, size_t NodeMemberOffset) {
    if (!TreePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)TreePtr - NodeMemberOffset);
}
#define XRBT_ENTRY(_What, Type, Member) ((Type*)(XRBT_Cast((_What), offsetof(Type, Member))))

static inline XelRBNode * XRBT_First(XelRBTree * TreePtr)
{
    return XRBN_LeftMost(TreePtr->RootPtr);
}

static inline XelRBNode * XRBT_Last(XelRBTree * TreePtr)
{
    return XRBN_RightMost(TreePtr->RootPtr);
}

static inline XelRBNode *XRBT_Find(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr) {
    XelRBNode * CurrNodePtr = TreePtr->RootPtr;
    while (CurrNodePtr) {
        int CompareResult = (*CompFunc)(TreePtr, KeyPtr, CurrNodePtr);
        if (CompareResult < 0) {
            CurrNodePtr = CurrNodePtr->LeftNodePtr;
        }
        else if (CompareResult > 0) {
            CurrNodePtr = CurrNodePtr->RightNodePtr;
        }
        else {
            return CurrNodePtr;
        }
    }
    return NULL;
}

typedef struct XelRBInsertNode
{
    XelRBNode * ParentPtr;
    XelRBNode ** SubNodeRefPtr;
} XelRBInsertNode;

static inline XelRBInsertNode XRBT_FindInsertSlot(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void *KeyPtr) {
    XelRBInsertNode InsertNode = {};
    XelRBNode ** CurrNodeRefPtr = &TreePtr->RootPtr;
    while (*CurrNodeRefPtr) {
        InsertNode.ParentPtr = *CurrNodeRefPtr;
        int CompareResult = (*CompFunc)(TreePtr, KeyPtr, *CurrNodeRefPtr);
        if (CompareResult < 0) {
            CurrNodeRefPtr = &(*CurrNodeRefPtr)->LeftNodePtr;
        }
        else if (CompareResult > 0) {
            CurrNodeRefPtr = &(*CurrNodeRefPtr)->RightNodePtr;
        }
        else {
            CurrNodeRefPtr = NULL;
            break;
        }
    }
    InsertNode.SubNodeRefPtr = CurrNodeRefPtr;
    return InsertNode;
}

typedef struct XelRBInsertResult
{
    bool Inserted;
    XelRBNode * PrevNode;
} XelRBInsertResult;

static inline XelRBInsertResult XRBT_Insert(XelRBTree * TreePtr, XelRBNode * NodePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr, bool AllowReplace)
{
    assert(!NodePtr->ParentPtr);
    assert(!NodePtr->LeftNodePtr);
    assert(!NodePtr->RightNodePtr);
    assert(!NodePtr->Flags);
    XelRBInsertResult Result = {};

    XelRBInsertNode InsertNode = XRBT_FindInsertSlot(TreePtr, CompFunc, KeyPtr);
    if (!InsertNode.ParentPtr) { // root
        assert(!TreePtr->RootPtr);
        TreePtr->RootPtr = NodePtr;
        Result.Inserted = true;
        return Result;
    }

    if (!InsertNode.SubNodeRefPtr) { // Replacement
        if (!AllowReplace) {
            Result.PrevNode = InsertNode.ParentPtr;
            return Result;
        }
        XelRBNode * ReplaceNodePtr = InsertNode.ParentPtr;
        if ((NodePtr->LeftNodePtr = ReplaceNodePtr->LeftNodePtr)) {
            NodePtr->LeftNodePtr->ParentPtr = NodePtr;
        }
        if ((NodePtr->RightNodePtr = ReplaceNodePtr->RightNodePtr)) {
            NodePtr->RightNodePtr->ParentPtr = NodePtr;
        }
        NodePtr->Flags = ReplaceNodePtr->Flags;

        if ((NodePtr->ParentPtr = ReplaceNodePtr->ParentPtr)) {
            XelRBNode * ParentNodePtr = NodePtr->ParentPtr;
            if (ParentNodePtr->LeftNodePtr == ReplaceNodePtr) {
                ParentNodePtr->LeftNodePtr = NodePtr;
            } else {
                ParentNodePtr->RightNodePtr = NodePtr;
            }
        } else { // root
            assert(TreePtr->RootPtr == ReplaceNodePtr);
            TreePtr->RootPtr = NodePtr;
        }
        XRBN_Init(ReplaceNodePtr);
        Result.Inserted = true;
        Result.PrevNode = ReplaceNodePtr;
        return Result;
    }

    *InsertNode.SubNodeRefPtr = NodePtr;
    NodePtr->ParentPtr = InsertNode.ParentPtr;
    // TODO: fix: (rb-rebalance)


    // return insert done
    Result.Inserted = true;
    return Result;
}

#define XRBT_FOR_EACH(_iter, _tree) \
    for (XelRBNode *_iter = XRBT_First((_tree)); _iter; _iter = XRBN_Next(_iter))

#define XRBT_FOR_EACH_SAFE(_iter, _tree) \
    for (XelRBNode *_iter = XRBT_First((_tree)), *_safe = XRBN_Next(_iter); _iter; _iter = _safe, _safe = XRBN_Next(_iter))

ZEC_API bool XRBT_Check(XelRBTree * TreePtr);

#ifdef __cplusplus
}
#endif
