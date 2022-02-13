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


static constexpr const size_t Total = 20;
static TestNode * NodePool[Total] = {};
static XelRBTree Tree;

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

void ClearAll()
{
    ClearNodePool();
    XRBT_Init(&Tree);
}

static int Compare(XelRBTree * TreePtr, const void * KeyPtr, XelRBNode * NodePtr)
{
    TestNode * TestNodePtr = XRBN_ENTRY(NodePtr, TestNode, Node);
    return (int)(*(size_t*)KeyPtr - TestNodePtr->Key);
}

TestNode * TreeInsert(size_t Index)
{
    if (NodePool[Index]) {
        return NULL;
    }
    auto TestNodePtr = new TestNode;
    TestNodePtr->Key = Index;
    XRBN_Init(&TestNodePtr->Node);
    XRBT_Insert(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key, false);

    NodePool[Index] = TestNodePtr;
    return TestNodePtr;
}

bool CheckOrder(XelRBTree * TreePtr)
{
    size_t Last = 0;
    XRBT_FOR_EACH(Iter, TreePtr) {
        size_t Key = XRBN_ENTRY(Iter, TestNode, Node)->Key;
        if (Key && Key <= Last) {
            return false;
        }
        Last = Key;
    }
    return true;
}

void test0()
{
    XRBT_Init(&Tree);
    srand(time(nullptr));

    size_t Counter = 0;
    std::vector<size_t> PushOrder;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        if (!Index || NodePool[Index]) {
            continue;
        }
        auto TestNodePtr = new TestNode {};
        TestNodePtr->Key = Index;
        XRBN_Init(&TestNodePtr->Node);
        NodePool[Index] = TestNodePtr;
        PushOrder.push_back(Index);
        XRBT_Insert(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key, false);
        ++Counter;
    }
    if (!CheckOrder(&Tree)) {
        cerr << "RB order error" << endl;
        exit(-1);
    }
    if (!XRBT_Check(&Tree)) {
        cerr << "RB balance error" << endl;
        exit(-1);
    }

    ClearAll();
}

void test1()
{
    XRBT_Init(&Tree);

    TreeInsert(5);
    for (size_t i = 0 ; i < Total; ++i) {
        TestNode * TestNodePtr = NodePool[i];
        if (!TestNodePtr) {
            continue;
        }
        XelRBNode * NodePtr = &TestNodePtr->Node;
        if (!NodePtr->RightNodePtr) {
            if (XRBN_IsRoot(NodePtr) || XRBN_IsRed(NodePtr)) {
                XRBT_Remove(&Tree, &TestNodePtr->Node);
            }
        }
    }
    if (!XRBT_Check(&Tree)) {
        cerr << "Remove Error" << endl;
        exit(-1);
    }

    ClearAll();
}

void test2()
{
    XRBT_Init(&Tree);

    srand(time(nullptr));

    TreeInsert(5);
    TreeInsert(3);
    for (size_t i = 0 ; i < Total; ++i) {
        TestNode * TestNodePtr = NodePool[i];
        if (!TestNodePtr) {
            continue;
        }
        XelRBNode * NodePtr = &TestNodePtr->Node;
        if (!NodePtr->RightNodePtr) {
            if (XRBN_IsRoot(NodePtr) || XRBN_IsRed(NodePtr)) {
                XRBT_Remove(&Tree, &TestNodePtr->Node);
            }
        }
    }
    if (!XRBT_Check(&Tree)) {
        cerr << "Remove Error" << endl;
        exit(-1);
    }

    ClearAll();
}

void test3()
{
    XRBT_Init(&Tree);

    srand(time(nullptr));
    size_t Counter = 0;
    std::vector<size_t> PushOrder;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        auto TestNodePtr = TreeInsert(Index) ;
        if (!TestNodePtr) {
            continue;
        }
        PushOrder.push_back(Index);
        ++Counter;
    }

    for (size_t i = 0 ; i < Total; ++i) {
        TestNode * TestNodePtr = NodePool[i];
        if (!TestNodePtr) {
            continue;
        }
        XelRBNode * NodePtr = &TestNodePtr->Node;
        if (!NodePtr->RightNodePtr) {
            if (XRBN_IsRoot(NodePtr) || XRBN_IsRed(NodePtr)) {
                XRBT_Remove(&Tree, &TestNodePtr->Node);
            }
        }
    }

    if (!XRBT_Check(&Tree)) {
        cerr << "Remove Error" << endl;
        exit(-1);
    }

    ClearAll();
}

void test4()
{
    XRBT_Init(&Tree);
    TreeInsert(2);
    TreeInsert(17);
    TreeInsert(4);
    TreeInsert(1);
    TreeInsert(16);
    TreeInsert(6);
    TreeInsert(11);
    TreeInsert(8);
    TreeInsert(5);
    TreeInsert(13);
    TreeInsert(12);
    TreeInsert(10);
    TreeInsert(15);
    TreeInsert(3);
    // PrintTree(&Tree);

    // cout << "=========" << endl;
    XRBT_Remove(&Tree, &NodePool[4]->Node);
    XRBT_Remove(&Tree, &NodePool[3]->Node);
    // PrintTree(&Tree);

    ClearAll();
}

void test5()
{
    XRBT_Init(&Tree);

    srand(time(nullptr));
    size_t Counter = 0;
    std::vector<size_t> PushOrder;
    for (size_t i = 0 ; i < Total; ++i) {
        size_t Index = rand() % Total;
        auto TestNodePtr = TreeInsert(Index) ;
        if (!TestNodePtr) {
            continue;
        }
        PushOrder.push_back(Index);
        ++Counter;
    }

    for(auto & i : PushOrder) {
        cout << "Push " << i << endl;
    }

    for (int i = 0 ; i < 10; ++i) {
        XelRBNode * NodeLeftPtr = Tree.RootPtr->LeftNodePtr;
        size_t Index = XRBN_ENTRY(NodeLeftPtr, TestNode, Node)->Key;
        PrintTree(&Tree);
        cout << "Remove: " << Index << endl;
        XRBT_Remove(&Tree, NodeLeftPtr);
        delete NodePool[Index];
        NodePool[Index] = NULL;
    }

    // cout << "===========" << endl;

    // for (size_t i = 0 ; i < Total; ++i) {
    //     TestNode * TestNodePtr = NodePool[i];
    //     if (!TestNodePtr) {
    //         continue;
    //     }
    //     XelRBNode * NodePtr = &TestNodePtr->Node;
    //     if (rand() % 2) {
    //         cout << "Remove: " << XRBN_ENTRY(NodePtr, TestNode, Node)->Key << endl;
    //         XRBT_Remove(&Tree, NodePtr);
    //     }
    // }

    if (!CheckOrder(&Tree)) {
        cerr << "RB order error" << endl;
        PrintTree(&Tree);
        exit(-1);
    }
    // if (!XRBT_Check(&Tree)) {
    //     cerr << "Remove Error" << endl;
    //     exit(-1);
    // }

    ClearAll();
}

int main(int, char **)
{
    try {
        test0();
        test1();
        test2();
        test3();
        test4();
        // test5();
    }
    catch (...) {
        return -1;
    }
    return 0;
}
