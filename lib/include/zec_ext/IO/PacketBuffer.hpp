#pragma once
#include "./Packet.hpp"

ZEC_NS
{

    class xPacketBuffer final
    : xNonCopyable
    {
    public:
        ubyte           Buffer[MaxPacketSize];
        size_t          DataSize = 0;

    public:
        ZEC_INLINE size_t Pop(void * DstDataPtr, size_t DstDataBufferSize) {
            auto CopySize = std::min(DataSize, DstDataBufferSize);
            memcpy(DstDataPtr, Buffer, CopySize);
            if(auto Remains = DataSize - CopySize) {
                memmove(Buffer, Buffer + CopySize, Remains);
            } else {
                DataSize = 0;
            }
            return CopySize;
        }
        ZEC_INLINE size_t Push(const void * SrcDataPtr, size_t SrcDataSize) {
            auto CopySize = std::min(sizeof(Buffer) - DataSize, SrcDataSize);
            memcpy(Buffer + DataSize, SrcDataPtr, CopySize);
            DataSize += CopySize;
            return CopySize;
        }

    private:
        xPacketBuffer *   NextBufferPtr = nullptr;
        friend class xPacketBufferQueue;
    };

    class xPacketBufferQueue final
    {
    public:
        ZEC_INLINE xPacketBuffer * Peek() {
            return _FirstPtr;
        }
        ZEC_INLINE xPacketBuffer * RemoveFront() {
            assert(_FirstPtr);
            if (!(_FirstPtr = Steal(_FirstPtr->NextBufferPtr))) {
                _LastPtr = nullptr;
            }
            --TotalBufferCount;
            return _FirstPtr;
        }
        ZEC_INLINE xPacketBuffer * Pop() {
            if (auto TargetPtr = Peek()) {
                RemoveFront();
                return TargetPtr;
            }
            return nullptr;
        }
        ZEC_INLINE void Push(xPacketBuffer * BufferPtr) {
            assert(BufferPtr);
            assert(!BufferPtr->NextBufferPtr);
            if (!_LastPtr) {
                _FirstPtr = _LastPtr = BufferPtr;
            } else {
                _LastPtr->NextBufferPtr = BufferPtr;
                _LastPtr = BufferPtr;
            }
            ++TotalBufferCount;
        }
        ZEC_INLINE size_t Push(const void * DataPtr, size_t DataSize) {
            if (!_LastPtr) {
                return 0;
            }
            return _LastPtr->Push(DataPtr, DataSize);
        }

        ZEC_INLINE size_t GetSize () const { return TotalBufferCount; }
        ZEC_INLINE size_t IsEmpty () const { return !GetSize(); }
        ZEC_INLINE xPacketBuffer * GetLast() const { return _LastPtr; }

    private:
        xPacketBuffer * _FirstPtr = nullptr;
        xPacketBuffer * _LastPtr = nullptr;
        size_t TotalBufferCount = 0;
    };

}