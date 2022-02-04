#pragma once
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XEL_RBNODE_RED    ((uint32_t)(0x01))
#define XEL_RBNODE_ROOT   ((uint32_t)(0x02))

/* Node */
typedef struct XelRBNode XelRBNode;
struct XelRBNode {
    void *        ParentPtr;
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

static inline bool XRBN_IsRed(XelRBNode * NodePtr) {
    return NodePtr->Flags & XEL_RBNODE_RED;
}
static inline bool XRBN_IsBlack(XelRBNode * NodePtr) {
    return !XRBN_IsRed(NodePtr);
}
static inline void XRBN_MarkRed(XelRBNode * NodePtr) {
    NodePtr->Flags |= XEL_RBNODE_RED;
}
static inline void XRBN_MarkBlack(XelRBNode * NodePtr) {
    NodePtr->Flags &= ~XEL_RBNODE_RED;
}

static inline bool XRBN_IsRoot(XelRBNode * NodePtr) {
    return NodePtr->Flags & XEL_RBNODE_ROOT;
}
static inline void XRBN_MarkRoot(XelRBNode * NodePtr) {
    NodePtr->Flags |= XEL_RBNODE_ROOT;
}
static inline void XRBN_RemoveRoot(XelRBNode * NodePtr) {
    NodePtr->Flags &= ~XEL_RBNODE_ROOT;
}

static inline void* XRBN_Cast(XelRBNode* NodePtr, size_t NodeMemberOffset) {
    if (!NodePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)NodePtr - NodeMemberOffset);
}

static inline bool XRBN_IsLinked(XelRBNode * NodePtr) {
    return NodePtr->ParentPtr;
}

static inline void XRBN_Unlink(XelRBNode * NodePtr) {
    if(!XRBN_IsLinked(NodePtr)) {
        // no need to unlink the root node
        return;
    }
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
    while ((ParentPtr = NodePtr->ParentPtr) && NodePtr == ParentPtr->LeftNodePtr) {
        NodePtr = ParentPtr;
    }
    return ParentPtr;
}

static inline XelRBNode * XRBN_Next(XelRBNode * NodePtr) {
    XelRBNode * ParentPtr;
    if (NodePtr->RightNodePtr) {
        return XRBN_LeftMost(NodePtr->RightNodePtr);
    }
    while ((ParentPtr = NodePtr->ParentPtr) && NodePtr == ParentPtr->RightNodePtr) {
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

typedef int XRBT_KeyCompare(XelRBTree * TreePtr, XelRBNode * NodePtr, const void * KeyPtr);

static inline void XRBT_Init(XelRBTree* TreePtr) {
    XelRBTree InitValue = {};
    *TreePtr = InitValue;
}

static inline bool XRBT_IsEmpty(XelRBTree* TreePtr) {
    return TreePtr->RootPtr;
}

static inline void* XRBT_Cast(XelRBTree* TreePtr, size_t NodeMemberOffset) {
    if (!TreePtr) {
        return NULL;
    }
    return (void*)((unsigned char*)TreePtr - NodeMemberOffset);
}
#define XRBT_ENTRY(_What, Type, Member) ((Type*)(XRBT_Cast((_What), offsetof(Type, Member))))

static inline XelRBNode * XRBN_First(XelRBTree * TreePtr)
{
    return XRBN_LeftMost(TreePtr->RootPtr);
}

static inline XelRBNode *XRBT_Find(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr) {
    XelRBNode * CurrNodePtr = TreePtr->RootPtr;
    while (CurrNodePtr) {
        int CompareResult = (*CompFunc)(TreePtr, CurrNodePtr, KeyPtr);
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
        int CompareResult = (*CompFunc)(TreePtr, *CurrNodeRefPtr, KeyPtr);
        InsertNode.ParentPtr = *CurrNodeRefPtr;
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

#define XRBT_FOR_EACH(_iter, _tree) \
    for (XelRBNode *_iter = c_rbtree_first(_tree); _iter; _iter = c_rbnode_next(_iter))

#define XRBT_FOR_EACH_SAFE(_iter, _tree) \
    for (XelRBNode *_iter = c_rbtree_first(_tree), *_safe = c_rbnode_next(_iter); _iter; _iter = _safe, _safe = c_rbnode_next(_iter))

#ifdef __cplusplus
}
#endif
