#include <xel/Util/IndexedStorage.hpp>
#include <iostream>
#include <cstring>

using namespace xel;
using namespace std;

#define TestSize 512

static void TestIdPool()
{
    xIndexIdPool Pool;
    if (!Pool.Init(TestSize)) {
        cerr << "TestIdPool: Failed to init pool" << endl;
        exit(-1);
    }
    xScopeGuard Cleaner = [&]{
        Pool.Clean();
    };

    uint64_t FailKey = xIndexId::InvalidValue;
    uint64_t Keys[TestSize];
    memset(Keys, 0, sizeof(Keys));

    if (Pool.Check(static_cast<uint64_t>(-1))) {
        cerr << "TestIdPool fatal: Attack key allowd" << endl;
        exit(-1);
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = Pool.Acquire();
        if (Keys[i] == xIndexId::InvalidValue) {
            cerr << "TestIdPool Failed to acquire index, IndexId=" << i << endl;
            exit(-1);
        }
    }

    FailKey = Pool.Acquire();
    if (FailKey != xIndexId::InvalidValue) {
        printf("TestIdPool Failed to fail on acquire over TestSize\n");
        exit(-1);
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        if (!Pool.CheckAndRelease(Keys[i])) {
            printf("TestIdPool Failed to check and release valid key, IndexId=%zi\n", i);
            exit(-1);
        }
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        if (Pool.CheckAndRelease(Keys[i])) {
            printf("TestIdPool Failed to ignore recycled index, IndexId=%zi\n", i);
            exit(-1);
        }
    }
    memset(Keys, 0, sizeof(Keys));

    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = Pool.Acquire();
        if (Keys[i] == xIndexId::InvalidValue) {
            cerr << "TestIdPool second phase failed to acquire index, IndexId=" << i << endl;
            exit(-1);
        }
    }

    FailKey = Pool.Acquire();
    if (FailKey != xIndexId::InvalidValue) {
        cerr << "TestIdPool Failed to fail on acquire over TestSize" << endl;
        exit(-1);
    }

    return;
}

struct xCounter
{
    xCounter() {
        ++Total;
    }
    xCounter(size_t I) : Index{I} {
        ++Total;
    };
    ~xCounter() {
        --Total;
    }
    xCounter(const xCounter & Other) : Index { Other.Index } {
        ++Total;
    }

    size_t Index = 0;
    static size_t Total;
};
size_t xCounter::Total = 0;

static void TestIdStorage()
{
    xIndexedStorage<xCounter> Pool;
    if (!Pool.Init(TestSize)) {
        cerr << "TestIdStorage failed to init pool" << endl;
        exit(-1);
    }
    xScopeGuard Cleaner = [&] {
        Pool.Clean();
        if (xCounter::Total != 0) {
            cerr << "Failed to clean all stored objects" << endl;
            exit(-1);
        }
    };

    uint64_t FailKey = xIndexId::InvalidValue;
    uint64_t Keys[TestSize];
    memset(Keys, 0, sizeof(Keys));
    if (Pool.Check(static_cast<uint64_t>(-1))) {
        cerr << "TestIdPool fatal: Attack key allowd" << endl;
        exit(-1);
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = Pool.Acquire(xCounter(i));
        if (Keys[i] == xIndexId::InvalidValue) {
            cerr << "TestIdStorage Failed to acquire index, IndexId=" << i << endl;
            exit(-1);
        }
    }
    if (xCounter::Total != TestSize) {
        cerr << "TestIdStorage invalid counter" << endl;
        exit(-1);
    }

    FailKey = Pool.Acquire(xCounter(-1));
    if (FailKey != xIndexId::InvalidValue) {
        cerr << "TestIdStorage failed to fail on index over TestSize" << endl;
        exit(-1);
    }

    for (size_t i = 0 ; i < TestSize; ++i) {
        if (Keys[i] == xIndexId::InvalidValue) {
            cerr << "TestIdStorage Failed to acquire index, IndexId=" << i << endl;
            exit(-1);
        }
        auto Opt = Pool.CheckAndGet(Keys[i]);
        if (!Opt() || Opt->Index != i) {
            cerr << "TestIdStorage faild to get oringally stored value index=" << i << ", value=" << Opt->Index << endl;
            exit(-1);
        }
        auto OptRelease = Pool.CheckAndRelease(Keys[i]);
        if (!OptRelease()) {
            cerr << "TestIdStorage faild to check and release, index=%" << i << endl;
            exit(-1);
        }
    }
    if (xCounter::Total != 0) {
        cerr << "TestIdStorage redundant objects" << endl;
        exit(-1);
    }
    for (size_t i = 0 ; i < TestSize; ++i) {
        Keys[i] = Pool.Acquire(xCounter(i));
        if (Keys[i] == xIndexId::InvalidValue) {
            cerr << "TestIdStorage second phase failed to acquire index, IndexId=" << i << endl;
            exit(-1);
        }
    }
    FailKey = Pool.Acquire(xCounter(-1));
    if (FailKey != xIndexId::InvalidValue) {
        cerr << "TestIdStorage second phase failed to fail on index over TestSize" << endl;
        exit(-1);
    }
}

int main(int, char **)
{
    TestIdPool();
    TestIdStorage();
    return 0;
}
