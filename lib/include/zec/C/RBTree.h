#pragma once
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    assert(XRBN_IsLinked(NodePtr));
    XRBN_UnlinkStale(NodePtr);
    XRBN_Init(NodePtr);
}

#define XEL_RBNODE_RED    ((uint32_t)(0x01))
#define XEL_RBNODE_ROOT   ((uint32_t)(0x02))
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

static inline XelRBNode *XRBT_Find(XelRBTree * TreePtr, XRBT_KeyCompare * Func, const void * KeyPtr) {
    XelRBNode * CurrNodePtr = TreePtr->RootPtr;
    while (CurrNodePtr) {
        int CompareResult = (*Func)(TreePtr, CurrNodePtr, KeyPtr);
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


#ifdef __cplusplus
}
#endif
