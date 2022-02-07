#include <zec/Common.hpp>
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

static constexpr const size_t Total = 3;
static TestNode * NodePool[Total] = {};

size_t GenerateNodePool()
{
    srand(time(nullptr));
    size_t Counter = 0;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        if (NodePool[Index]) {
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
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        switch(i) {
            case 0:
                Index = 3; break;
            case 1:
                Index = 1; break;
            case 2:
                Index = 2; break;
            default:
            break;
        }
        if (NodePool[Index]) {
            continue;
        }

        // cout << "Trying to insert " << Index << endl;
        if (Index > MaxKey) {
            MaxKey = Index;
        }

        auto TestNodePtr = new TestNode {};
        TestNodePtr->Key = Index;
        XRBN_Init(&TestNodePtr->Node);

        XRBT_Insert(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key, false);

        NodePool[Index] = TestNodePtr;
        ++Counter;
    }
    cout << "GeneratedCount: " << Counter << endl;
    cout << "MaxKey: " << MaxKey << endl;

    size_t Last = 0;
    Counter = 0;
    XRBT_FOR_EACH(Iter, &Tree) {
        size_t Key = XRBN_ENTRY(Iter, TestNode, Node)->Key;
        cout << "Counter: " << Counter << ", Key: " << Key << ", MaxKey:" << MaxKey << endl;
        if (Key && Key <= Last) {
            cerr << ("TreeOrderError") << endl;
            continue;
        }
        Last = Key;
    }

    if (!XRBT_Check(&Tree)) {
        cout << "XRBT_Check failed" << endl;
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
