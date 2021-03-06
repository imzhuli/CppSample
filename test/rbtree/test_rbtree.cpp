#include <zec/Common.hpp>
#include <vector>
#include <cmath>
#include <ctime>
#include <iostream>
#include <zec/C/ZEC_RBTree.h>

using namespace std;
using namespace zec;

struct TestNode
{
    XelRBNode Node;
    size_t Key;
};


static constexpr const size_t Total = 102400;
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
    auto PrevNodePtr = XRBT_InsertOrAssign(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key);
    NodePool[Index] = TestNodePtr;
    if (PrevNodePtr) {
        delete PrevNodePtr;
    }
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
        XRBT_InsertOrAssign(&Tree, &TestNodePtr->Node, &Compare, &TestNodePtr->Key);
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
    TreeInsert(7);
    TreeInsert(18);
    TreeInsert(19);
    TreeInsert(20);
    TreeInsert(12);

    TreeInsert(17);
    TreeInsert(22);
    TreeInsert(23);
    TreeInsert(4);
    TreeInsert(10);

    TreeInsert(24);
    TreeInsert(14);
    TreeInsert(5);
    TreeInsert(16);
    TreeInsert(0);

    TreeInsert(2);
    TreeInsert(11);

    cout << "=========" << endl;
    XRBT_Remove(&Tree, &NodePool[7]->Node);
    XRBT_Remove(&Tree, &NodePool[5]->Node);
    XRBT_Remove(&Tree, &NodePool[4]->Node);
    XRBT_Remove(&Tree, &NodePool[2]->Node);
    XRBT_Remove(&Tree, &NodePool[0]->Node);

    XRBT_Remove(&Tree, &NodePool[12]->Node);
    XRBT_Remove(&Tree, &NodePool[11]->Node);
    XRBT_Remove(&Tree, &NodePool[16]->Node);

    PrintTree(&Tree);
    XRBT_Remove(&Tree, &NodePool[18]->Node);
    XRBT_Remove(&Tree, &NodePool[14]->Node);


    if (!XRBT_Check(&Tree)) {
        cerr << "Remove Error" << endl;
        exit(-1);
    }

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

    for (size_t i = 0 ; i < Total; ++i) {
        TestNode * TestNodePtr = NodePool[i];
        if (!TestNodePtr) {
            continue;
        }
        XelRBNode * NodePtr = &TestNodePtr->Node;
        if (rand() % 2) {
            cout << "Remove: " << XRBN_ENTRY(NodePtr, TestNode, Node)->Key << endl;
            XRBT_Remove(&Tree, NodePtr);
        }
    }

    if (!CheckOrder(&Tree)) {
        cerr << "RB order error" << endl;
        PrintTree(&Tree);
        exit(-1);
    }
    if (!XRBT_Check(&Tree)) {
        cerr << "Remove Error" << endl;
        exit(-1);
    }

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
        test5();
    }
    catch (...) {
        return -1;
    }
    return 0;
}
