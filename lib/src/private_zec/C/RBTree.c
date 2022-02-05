#include <zec/C/RBTree.h>

static size_t XRBN_CountBlack(XelRBNode * RootPtr, XelRBNode * NodePtr)
{
    size_t Counter = 0;
    while(NodePtr != RootPtr) {
        if (XRBN_IsBlack(NodePtr)) {
            ++Counter;
        }
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
        YPtr->ParentPtr = YPtr;
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
        YPtr->ParentPtr = YPtr;
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
    YPtr->RightNodePtr = XPtr;
}


void XRBN_UnlinkStale(XelRBNode * NodePtr)
{
    assert(false);
}
