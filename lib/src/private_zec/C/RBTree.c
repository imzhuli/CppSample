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

void XRBN_LeftRotate(XelRBNode * NodePtr)
{
    XelRBNode * XPtr = NodePtr;
    XelRBNode * YPtr = NodePtr->RightNodePtr;
    XelRBNode * SubPtr = YPtr->LeftNodePtr;
    if ((XPtr->RightNodePtr = SubPtr)) {
        SubPtr->ParentPtr = XPtr;
    }

    XelRBNode * ParentPtr = XPtr->ParentPtr;
    YPtr->ParentPtr = ParentPtr;
    if (ParentPtr->LeftNodePtr == XPtr) {
        ParentPtr->LeftNodePtr = YPtr;
    } else {
        ParentPtr->RightNodePtr = YPtr;
    }

    XPtr->ParentPtr = YPtr;
    YPtr->LeftNodePtr = XPtr;
}

void XRBN_RightRotate(XelRBNode * NodePtr)
{
    XelRBNode * XPtr = NodePtr;
    XelRBNode * YPtr = NodePtr->LeftNodePtr;
    XelRBNode * SubPtr = YPtr->RightNodePtr;
    if ((XPtr->LeftNodePtr = SubPtr)) {
        SubPtr->ParentPtr = XPtr;
    }

    XelRBNode * ParentPtr = XPtr->ParentPtr;
    YPtr->ParentPtr = ParentPtr;
    if (ParentPtr->RightNodePtr == XPtr) {
        ParentPtr->RightNodePtr = YPtr;
    } else {
        ParentPtr->LeftNodePtr = YPtr;
    }

    XPtr->ParentPtr = YPtr;
    YPtr->RightNodePtr = XPtr;
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
    // assert(!NodePtr->ParentPtr);
    // assert(!NodePtr->LeftNodePtr);
    // assert(!NodePtr->RightNodePtr);
    // assert(!NodePtr->RedFlag);
    XelRBInsertResult Result = {};

    XelRBInsertNode InsertNode = XRBT_FindInsertSlot(TreePtr, CompFunc, KeyPtr);
    if (!InsertNode.ParentPtr) { // root
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
    for(XelRBNode * FP = FixNodePtr->ParentPtr; XRBN_IsGenericRed(FP); FP = FixNodePtr->ParentPtr) {
        XelRBNode * FPP = FP->ParentPtr;
        if (FP == FPP->LeftNodePtr) {
            XelRBNode * FPPR = FPP->RightNodePtr;
            if (XRBN_IsGenericRed(FPPR)) {
                XRBN_MarkBlack(FP);
                XRBN_MarkBlack(FPPR);
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
                XRBN_MarkBlack(FPPL);
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

static inline XelRBNode * XRBT_Maximum(XelRBNode * NodePtr)
{
    while(NodePtr->RightNodePtr) {
        NodePtr = NodePtr->RightNodePtr;
    }
    return NodePtr;
}

void XRBT_Fix(XelRBTree * TreePtr, XelRBNode * FixNodePtr, XelRBNode * ParentPtr, bool Left)
{
    while(true) {
        if (Left) {
            XelRBNode * SiblingPtr = ParentPtr->RightNodePtr;
            if (XRBN_IsBlack(ParentPtr) && XRBN_IsRed(SiblingPtr)) { // case 1
                XRBT_LeftRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(SiblingPtr);
                XRBN_MarkRed(ParentPtr);
                SiblingPtr = ParentPtr->RightNodePtr;
            }

            XelRBNode * RNephew = SiblingPtr->RightNodePtr;
            if (XRBN_IsGenericRed(RNephew)) { // case 4
                SiblingPtr->RedFlag = ParentPtr->RedFlag;
                XRBT_LeftRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(ParentPtr);
                XRBN_MarkBlack(RNephew);
                XRBN_MarkBlack(TreePtr->RootPtr);
                return;
            }

            XelRBNode * LNephew = SiblingPtr->LeftNodePtr;
            if (XRBN_IsGenericRed(LNephew)) { // case 3
                LNephew->RedFlag = ParentPtr->RedFlag;
                XRBN_MarkBlack(ParentPtr);
                XRBN_RightRotate(SiblingPtr);
                XRBT_LeftRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(TreePtr->RootPtr);
                return;
            }

            XRBN_MarkRed(SiblingPtr);
        }
        else {
            XelRBNode * SiblingPtr = ParentPtr->LeftNodePtr;
            if (XRBN_IsBlack(ParentPtr) && XRBN_IsRed(SiblingPtr)) { // case 1
                XRBT_RightRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(SiblingPtr);
                XRBN_MarkRed(ParentPtr);
                SiblingPtr = ParentPtr->LeftNodePtr;
            }

            XelRBNode * LNephew = SiblingPtr->LeftNodePtr;
            if (XRBN_IsGenericRed(LNephew)) { // case 4
                SiblingPtr->RedFlag = ParentPtr->RedFlag;
                XRBT_RightRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(ParentPtr);
                XRBN_MarkBlack(LNephew);
                XRBN_MarkBlack(TreePtr->RootPtr);
                return;
            }

            XelRBNode * RNephew = SiblingPtr->RightNodePtr;
            if (XRBN_IsGenericRed(RNephew)) { // case 3
                RNephew->RedFlag = ParentPtr->RedFlag;
                XRBN_MarkBlack(ParentPtr);

                XRBN_LeftRotate(SiblingPtr);
                XRBT_RightRotate(TreePtr, ParentPtr);
                XRBN_MarkBlack(TreePtr->RootPtr);
                return;
            }

            XRBN_MarkRed(SiblingPtr);
        }

        if (XRBN_IsRed(ParentPtr)) {
            XRBN_MarkBlack(ParentPtr); // case 2
            return;
        };
        FixNodePtr = ParentPtr;
        if (!(ParentPtr = FixNodePtr->ParentPtr)) {
            return;
        }
        Left = (ParentPtr->LeftNodePtr == FixNodePtr);
        continue;
    }
}

void XRBT_Remove(XelRBTree * TreePtr, XelRBNode * NodePtr)
{
    // Phase 1: replace
    XelRBNode * PPtr = NodePtr->ParentPtr;
    XelRBNode * LPtr = NodePtr->LeftNodePtr;
    XelRBNode * RPtr = NodePtr->RightNodePtr;
    if (!RPtr) {
        if (!PPtr) { // remove root node
            if ((TreePtr->RootPtr = LPtr)) {
                LPtr->ParentPtr = NULL;
                XRBN_MarkBlack(LPtr);
            }
            XRBN_Init(NodePtr);
            return;
        }
        // move to parent
        bool Left = NodePtr == PPtr->LeftNodePtr;
        if (Left) {
            PPtr->LeftNodePtr = LPtr;
        } else {
            PPtr->RightNodePtr = LPtr;
        }
        // check and fix
        if (LPtr) {
            LPtr->ParentPtr = PPtr;
            // *LPtr must be red, so just mark its color as *NodePtr
            LPtr->RedFlag = NodePtr->RedFlag;
            XRBN_Init(NodePtr);
            return;
        } else { // both children are null
            if (XRBN_IsRed(NodePtr)) {
                XRBN_Init(NodePtr);
                return;
            }
            XRBT_Fix(TreePtr, NULL, PPtr, Left);
            XRBN_Init(NodePtr);
            return;
        }
    }
    else if (!LPtr) {
        if (!PPtr) { // remove root node
            if ((TreePtr->RootPtr = RPtr)) {
                RPtr->ParentPtr = NULL;
                XRBN_MarkBlack(RPtr);
            }
            XRBN_Init(NodePtr);
            return;
        }
        // move to parent
        if (NodePtr == PPtr->LeftNodePtr) {
            PPtr->LeftNodePtr = RPtr;
        } else {
            PPtr->RightNodePtr = RPtr;
        }
        RPtr->ParentPtr = PPtr;
        RPtr->RedFlag = NodePtr->RedFlag;
        XRBN_Init(NodePtr);
        return;
    }

    // both children exist:
    XelRBNode * ReplacePtr = XRBN_RightMost(LPtr);
    XelRBNode * SubPtr = ReplacePtr->LeftNodePtr;

    // move replace node to target node:
    if (ReplacePtr == LPtr) {
        if (!PPtr) { // root
            TreePtr->RootPtr = ReplacePtr;
            ReplacePtr->ParentPtr = NULL;
            ReplacePtr->RightNodePtr = RPtr;
            RPtr->ParentPtr = ReplacePtr;
        } else {
            if (PPtr->LeftNodePtr == NodePtr) {
                PPtr->LeftNodePtr = ReplacePtr;
            } else {
                PPtr->RightNodePtr = ReplacePtr;
            }
            ReplacePtr->ParentPtr = PPtr;
            ReplacePtr->RightNodePtr = RPtr;
            RPtr->ParentPtr = ReplacePtr;
        }
        if (SubPtr) {
            // assert(XRBN_IsRed(SubPtr));
            SubPtr->RedFlag = ReplacePtr->RedFlag;
            ReplacePtr->RedFlag = NodePtr->RedFlag;
            XRBN_Init(NodePtr);
            return;
        }
        if (XRBN_IsRed(ReplacePtr)) {
            ReplacePtr->RedFlag = NodePtr->RedFlag;
            XRBN_Init(NodePtr);
            return;
        }
        ReplacePtr->RedFlag = NodePtr->RedFlag;
        XRBN_Init(NodePtr);
        XRBT_Fix(TreePtr, NULL, ReplacePtr, true);
    }
    else {
        XelRBNode * ReplacePPtr = ReplacePtr->ParentPtr;
        if (ReplacePPtr->LeftNodePtr == ReplacePtr) {
            ReplacePPtr->LeftNodePtr = SubPtr;
        } else {
            ReplacePPtr->RightNodePtr = SubPtr;
        }

        if (!PPtr) { // root
            TreePtr->RootPtr = ReplacePtr;
            ReplacePtr->ParentPtr = NULL;
            ReplacePtr->LeftNodePtr = LPtr;
            LPtr->ParentPtr = ReplacePtr;
            ReplacePtr->RightNodePtr = RPtr;
            RPtr->ParentPtr = ReplacePtr;
        } else {
            if (PPtr->LeftNodePtr == NodePtr) {
                PPtr->LeftNodePtr = ReplacePtr;
            } else {
                PPtr->RightNodePtr = ReplacePtr;
            }
            ReplacePtr->ParentPtr = PPtr;
            ReplacePtr->LeftNodePtr = LPtr;
            LPtr->ParentPtr = ReplacePtr;
            ReplacePtr->RightNodePtr = RPtr;
            RPtr->ParentPtr = ReplacePtr;
        }
        if (SubPtr) {
            // assert(XRBN_IsRed(SubPtr));
            SubPtr->ParentPtr = ReplacePPtr;
            SubPtr->RedFlag = ReplacePtr->RedFlag;
            ReplacePtr->RedFlag = NodePtr->RedFlag;
            XRBN_Init(NodePtr);
            return;
        }
        if (XRBN_IsRed(ReplacePtr)) {
            ReplacePtr->RedFlag = NodePtr->RedFlag;
            XRBN_Init(NodePtr);
            return;
        }
        ReplacePtr->RedFlag = NodePtr->RedFlag;
        XRBN_Init(NodePtr);
        XRBT_Fix(TreePtr, NULL, ReplacePPtr, false);
    }
}
