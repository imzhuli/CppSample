#pragma once
#include "./MemoryPool.hpp"
#include "./IndexedStorage.hpp"

X_NS
{

    template<typename T>
    class xIndexedResourcePool
    {
    public:
        struct xResource
        {
            T *        InstancePtr;
            xIndexId   Index;
        };

        bool Init(size_t PoolSize)
        {
            xMemoryPoolOptions PoolOptions = {
                .InitSize          = 1'0000,
                .Addend            = 1'0000,
                .MaxSizeIncrement  = std::max(size_t(1'0000), PoolSize / 10),
            };
            PoolOptions.MaxPoolSize = PoolSize;
                if (!Pool.Init(PoolOptions)) {
                    return false;
                }
            if (!KeyManager.Init(PoolSize)) {
                Pool.Clean();
                return false;
            }
            return true;
        }

        void Clean() {
            KeyManager.Clean();
            Pool.Clean();
        }

        X_INLINE T* GetInstanceByKey(xIndexId Key) const {
            return *KeyManager.CheckAndGet(Key);
        }

        X_INLINE xResource Create() {
            auto InstancePtr = Pool.Create();
            if (!InstancePtr) {
                return { nullptr, xIndexId::InvalidValue };
            }
            return { InstancePtr, KeyManager.Acquire(InstancePtr) };
        }

        X_INLINE void Destroy(const xResource & Resource) {
            auto Key = Resource.Index;
            auto InstancePtr = Resource.InstancePtr;
            Destroy(Key, InstancePtr);
        }

        X_INLINE void Destroy(T* InstancePtr, xIndexId Key) {
            assert(InstancePtr == GetInstanceByKey(Key));
            KeyManager.Release(Steal(Key));
            Pool.Destroy(InstancePtr);
        }

    private:
        xIndexedStorage<T*>    KeyManager;
        xMemoryPool<T>         Pool;
    };

}