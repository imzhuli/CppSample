#include <zec/C/RBTree.h>

static size_t XRBN_CountBlack(XelRBNode * RootPtr, XelRBNode * NodePtr)
{
    size_t Counter = 0;
    while(NodePtr != RootPtr) {
        if (XRBN_IsBlack(NodePtr)) {
            ++Counter;
        }
        NodePtr = NodePtr->ParentPtr;
    }
    return Counter;
}

static bool XRBT_PathCheck(XelRBNode * NodePtr)
{
    XelRBNode * LeftMost = XRBN_LeftMost(NodePtr);
    XelRBNode * RightMost = XRBN_RightMost(NodePtr);
    size_t CheckCount = XRBN_CountBlack(NodePtr, RightMost);
    for (XelRBNode * Iter = LeftMost; Iter != RightMost; Iter = XRBN_Next(Iter)) {
        if (Iter->LeftNodePtr || Iter->RightNodePtr) {
            continue;
        }
        if (XRBN_CountBlack(NodePtr, Iter) != CheckCount) {
            return false;
        }
    }
    return true;
}

bool XRBT_Check(XelRBTree * TreePtr)
{
    XelRBNode * RootPtr = TreePtr->RootPtr;
    if (!RootPtr) {
        return true;
    }
    if (!XRBN_IsBlack(RootPtr)) {
        return false;
    }

    for (XelRBNode * Iter = XRBT_First(TreePtr); Iter; Iter = XRBN_Next(Iter)) {
        if (XRBN_IsRed(Iter)) {
            if (!XRBN_IsGenericBlack(Iter->LeftNodePtr) || !XRBN_IsGenericBlack(Iter->RightNodePtr)) {
                return false;
            }
        }
        if (!XRBT_PathCheck(Iter)) {
            return false;
        }
    }

    return true;
}

void XRBT_LeftRotate(XelRBTree * TreePtr, XelRBNode * NodePtr)
{
    XelRBNode * XPtr = NodePtr;
    XelRBNode * YPtr = NodePtr->RightNodePtr;
    XelRBNode * SubPtr = YPtr->LeftNodePtr;
    if ((XPtr->RightNodePtr = SubPtr)) {
        SubPtr->ParentPtr = XPtr;
    }
    if (TreePtr->RootPtr == XPtr) {
        TreePtr->RootPtr = YPtr;
        YPtr->ParentPtr = NULL;
    } else {
        XelRBNode * ParentPtr = XPtr->ParentPtr;
        YPtr->ParentPtr = ParentPtr;
        if (ParentPtr->LeftNodePtr == XPtr) {
            ParentPtr->LeftNodePtr = YPtr;
        } else {
            ParentPtr->RightNodePtr = YPtr;
        }
    }
    XPtr->ParentPtr = YPtr;
    YPtr->LeftNodePtr = XPtr;
}

void XRBT_RightRotate(XelRBTree * TreePtr, XelRBNode * NodePtr)
{
    XelRBNode * XPtr = NodePtr;
    XelRBNode * YPtr = NodePtr->LeftNodePtr;
    XelRBNode * SubPtr = YPtr->RightNodePtr;
    if ((XPtr->LeftNodePtr = SubPtr)) {
        SubPtr->ParentPtr = XPtr;
    }
    if (TreePtr->RootPtr == XPtr) {
        TreePtr->RootPtr = YPtr;
        YPtr->ParentPtr = NULL;
    } else {
        XelRBNode * ParentPtr = XPtr->ParentPtr;
        YPtr->ParentPtr = ParentPtr;
        if (ParentPtr->RightNodePtr == XPtr) {
            ParentPtr->RightNodePtr = YPtr;
        } else {
            ParentPtr->LeftNodePtr = YPtr;
        }
    }
    XPtr->ParentPtr = YPtr;
    YPtr->RightNodePtr = XPtr;
}

XelRBInsertResult XRBT_Insert(XelRBTree * TreePtr, XelRBNode * NodePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr, bool AllowReplace)
{
    assert(!NodePtr->ParentPtr);
    assert(!NodePtr->LeftNodePtr);
    assert(!NodePtr->RightNodePtr);
    assert(!NodePtr->RedFlag);
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
        NodePtr->RedFlag = ReplaceNodePtr->RedFlag;

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
    XRBN_MarkRed(NodePtr);

    // TODO: fix: (rb-rebalance)
    XelRBNode * FixNodePtr = NodePtr;
    for(XelRBNode * FP = FixNodePtr->ParentPtr; XRBN_IsGenericRed(FP);) {
        XelRBNode * FPP = FP->ParentPtr;
        if (FP == FPP->LeftNodePtr) {
            XelRBNode * FPPR = FPP->RightNodePtr;
            if (XRBN_IsGenericRed(FPPR)) {
                XRBN_MarkBlack(FP);
                XRBN_MarkRed(FPPR);
                XRBN_MarkRed(FPP);
                FixNodePtr = FPP;
            }
            else {
                if (FixNodePtr == FP->RightNodePtr) {
                    FixNodePtr = FP;
                    XRBT_LeftRotate(TreePtr, FixNodePtr);
                    FP = FixNodePtr->ParentPtr;
                    FPP = FP->ParentPtr;
                }
                XRBN_MarkBlack(FP);
                XRBN_MarkRed(FPP);
                XRBT_RightRotate(TreePtr, FPP);
            }
        }
        else {
            XelRBNode * FPPL = FPP->LeftNodePtr;
            if (XRBN_IsGenericRed(FPPL)) {
                XRBN_MarkBlack(FP);
                XRBN_MarkRed(FPPL);
                XRBN_MarkRed(FPP);
                FixNodePtr = FPP;
            }
            else {
                if (FixNodePtr == FP->LeftNodePtr) {
                    FixNodePtr = FP;
                    XRBT_RightRotate(TreePtr, FixNodePtr);
                    FP = FixNodePtr->ParentPtr;
                    FPP = FP->ParentPtr;
                }
                XRBN_MarkBlack(FP);
                XRBN_MarkRed(FPP);
                XRBT_LeftRotate(TreePtr, FPP);
            }
        }
    }
    XRBN_MarkBlack(TreePtr->RootPtr);

    // return insert done
    Result.Inserted = true;
    return Result;
}

static inline XelRBNode * XRBT_Minimum(XelRBNode * NodePtr)
{
    while(NodePtr->LeftNodePtr) {
        NodePtr = NodePtr->LeftNodePtr;
    }
    return NodePtr;
}

static inline void XRBT_TransplantStale(XelRBTree * TreePtr, XelRBNode * TargetNodePtr, XelRBNode * SubTreeNodePtr)
{
    XelRBNode * ParentPtr = TargetNodePtr->ParentPtr->LeftNodePtr;
    if (!ParentPtr) {
        assert(TargetNodePtr == TreePtr->RootPtr);
        TreePtr->RootPtr = SubTreeNodePtr;
    }
    else if (TargetNodePtr == ParentPtr->LeftNodePtr) {
        ParentPtr->LeftNodePtr = SubTreeNodePtr;
    }
    else { // right
        ParentPtr->RightNodePtr = SubTreeNodePtr;
    }
    SubTreeNodePtr->ParentPtr = ParentPtr;
}

void XRBT_Remove(XelRBTree * TreePtr, XelRBNode * NodePtr)
{
    bool IsPrevNodeRed = XRBN_IsRed(NodePtr);
    XelRBNode * FixNodePtr = NULL;
    if (!NodePtr->LeftNodePtr) {
        FixNodePtr = NodePtr->RightNodePtr;
        XRBT_TransplantStale(TreePtr, NodePtr, FixNodePtr);
    } else if (!NodePtr->RightNodePtr) {
        FixNodePtr = NodePtr->LeftNodePtr;
        XRBT_TransplantStale(TreePtr, NodePtr, FixNodePtr);
    } else {
        XelRBNode * SubTreePtr = XRBT_Minimum(NodePtr->RightNodePtr);
        IsPrevNodeRed = XRBN_IsRed(SubTreePtr);

        FixNodePtr = SubTreePtr->RightNodePtr;
        if (NodePtr == SubTreePtr->ParentPtr) {
            FixNodePtr->RightNodePtr = SubTreePtr;
        } else {
            XRBT_TransplantStale(TreePtr, SubTreePtr, SubTreePtr->RightNodePtr);
            XelRBNode * SubTreeRightPtr = NodePtr->RightNodePtr;
            SubTreePtr->RightNodePtr = SubTreeRightPtr;
            SubTreeRightPtr->ParentPtr = SubTreePtr;
        }
        XRBT_TransplantStale(TreePtr, NodePtr, SubTreePtr);

        XelRBNode * SubTreeLeftPtr = NodePtr->LeftNodePtr;
        SubTreePtr->LeftNodePtr = SubTreeLeftPtr;
        SubTreeLeftPtr->ParentPtr = SubTreePtr;
        SubTreePtr->RedFlag = NodePtr->RedFlag;
    }

    if (!IsPrevNodeRed) {
        // delete - fix


    }


    XRBN_Init(NodePtr);
}
