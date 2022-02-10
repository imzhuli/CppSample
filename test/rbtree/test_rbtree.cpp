#include <zec/Common.hpp>
#include <vector>
#include <cmath>
#include <ctime>
#include <iostream>
#include <zec/C/RBTree.h>

using namespace std;
using namespace zec;

struct TestNode
{
    XelRBNode Node;
    size_t Key;
};

static constexpr const size_t Total = 10240;
static TestNode * NodePool[Total] = {};

size_t GenerateNodePool()
{
    srand(time(nullptr));
    size_t Counter = 0;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        if (!Index || NodePool[Index]) {
            continue;
        }
        auto TestNodePtr = new TestNode {};
        TestNodePtr->Key = Index;
        XRBN_Init(&TestNodePtr->Node);

        NodePool[Index] = TestNodePtr;
        ++Counter;
    }
    return Counter;
}

void PrintTree(XelRBTree * TreePtr) {
    XRBT_FOR_EACH(Iter, TreePtr) {
        TestNode * NodePtr = XRBN_ENTRY(Iter, TestNode, Node);
        TestNode * ParentNodePtr = XRBN_ENTRY(Iter->ParentPtr, TestNode, Node);
        TestNode * LeftNodePtr = XRBN_ENTRY(Iter->LeftNodePtr, TestNode, Node);
        TestNode * RightNodePtr = XRBN_ENTRY(Iter->RightNodePtr, TestNode, Node);
        printf("Node(%c):%zi    P:%zi    L:%zi,    R:%zi\n",
            (XRBN_IsRed(Iter) ? 'R' : 'B'),
            NodePtr->Key,
            (ParentNodePtr ? ParentNodePtr->Key : 0),
            (LeftNodePtr ? LeftNodePtr->Key : 0),
            (RightNodePtr ? RightNodePtr->Key : 0)
            );
    }
}

void ClearNodePool()
{
    for (size_t i = 0 ;i < Total; ++i) {
        if (NodePool[i]) {
            delete NodePool[i];
            NodePool[i] = nullptr;
        }
    }
}

static XelRBTree Tree;

static int Compare(XelRBTree * TreePtr, const void * KeyPtr, XelRBNode * NodePtr)
{
    TestNode * TestNodePtr = XRBN_ENTRY(NodePtr, TestNode, Node);
    return (int)(*(size_t*)KeyPtr - TestNodePtr->Key);
}

void test1()
{
    XRBT_Init(&Tree);

    srand(time(nullptr));
    size_t Counter = 0;
    size_t MaxKey = 0;
    std::vector<size_t> PushOrder;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        // cout << "Trying to insert " << Index << endl;
        if (!Index || NodePool[Index]) {
            continue;
        }
        if (Index > MaxKey) {
            MaxKey = Index;
        }
        auto TestNodePtr = new TestNode {};
        TestNodePtr->Key = Index;
        XRBN_Init(&TestNodePtr->Node);

        NodePool[Index] = TestNodePtr;
        PushOrder.push_back(Index);
        XRBT_Insert(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key, false);

        // cout << "Push:" << Index << endl;
        // PrintTree(&Tree);
        // if (!XRBT_Check(&Tree)) {
        //     exit(-1);
        // }

        ++Counter;
    }
    // cout << "GeneratedCount: " << Counter << endl;
    // cout << "MaxKey: " << MaxKey << endl;

    size_t Last = 0;
    Counter = 0;
    XRBT_FOR_EACH(Iter, &Tree) {
        size_t Key = XRBN_ENTRY(Iter, TestNode, Node)->Key;
        // cout << "Counter: " << Counter << ", Key: " << Key << ", MaxKey:" << MaxKey << endl;
        if (Key && Key <= Last) {
            cerr << ("TreeOrderError") << endl;
            continue;
        }
        Last = Key;
    }

    ClearNodePool();
}

int main(int, char **)
{
    try {
        test1();
    }
    catch (...) {
        return -1;
    }
    return 0;
}
