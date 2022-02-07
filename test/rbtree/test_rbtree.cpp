#include <zec/Common.hpp>
#include <cmath>
#include <ctime>
#include <iostream>
#include <zec/C/RBTree.h>

using namespace std;
using namespace zec;

struct TestNode
{
    size_t Key;
    XelRBNode Node;
};

static constexpr const size_t Total = 202400;
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
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        if (NodePool[Index]) {
            continue;
        }

        auto TestNodePtr = new TestNode {};
        TestNodePtr->Key = Index;
        XRBN_Init(&TestNodePtr->Node);

        XRBT_Insert(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key, false);

        NodePool[Index] = TestNodePtr;
        ++Counter;
    }
    cout << "GeneratedCount: " << Counter << endl;

    size_t Last = 0;
    XRBT_FOR_EACH(Iter, &Tree) {
        size_t Key = XRBN_ENTRY(Iter, TestNode, Node)->Key;
        if (Key && Key <= Last) {
            Fatal("TreeOrderError");
            return;
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
