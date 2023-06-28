#pragma once
#include "./Packet.hpp"

X_NS
{

    class xPacketBuffer final
    : xNonCopyable
    {
    public:
        ubyte           Buffer[MaxPacketSize * 2];
        size_t          DataSize = 0;

    public:
        X_INLINE size_t Pop(void * DstDataPtr, size_t DstDataBufferSize) {
            auto CopySize = std::min(DataSize, DstDataBufferSize);
            if (X_LIKELY(CopySize)) {
                memcpy(DstDataPtr, Buffer, CopySize);
                if((DataSize -= CopySize)) {
                    memmove(Buffer, Buffer + CopySize, DataSize);
                }
            }
            return CopySize;
        }
        X_INLINE size_t Push(const void * SrcDataPtr, size_t SrcDataSize) {
            auto CopySize = std::min(sizeof(Buffer) - DataSize, SrcDataSize);
            if (X_LIKELY(CopySize)) {
                memcpy(Buffer + DataSize, SrcDataPtr, CopySize);
                DataSize += CopySize;
            }
            return CopySize;
        }
        X_INLINE bool HasNext() const { return NextBufferPtr; }

    private:
        xPacketBuffer * NextBufferPtr = nullptr;
        friend class xPacketBufferChain;
    };

    class xPacketBufferChain final
    : xNonCopyable
    {
    public:
        X_INLINE ~xPacketBufferChain()
        {
            assert(!_FirstPtr);
            assert(!_LastPtr);
            assert(!_TotalBufferCount);
        }

        X_INLINE xPacketBuffer * Peek() {
            return _FirstPtr;
        }
        X_INLINE void RemoveFront() {
            assert(_FirstPtr);
            if (!(_FirstPtr = Steal(_FirstPtr->NextBufferPtr))) {
                _LastPtr = nullptr;
            }
            --_TotalBufferCount;
        }
        X_INLINE xPacketBuffer * Pop() {
            if (auto TargetPtr = Peek()) {
                RemoveFront();
                return TargetPtr;
            }
            return nullptr;
        }
        X_INLINE void Push(xPacketBuffer * BufferPtr) {
            assert(BufferPtr);
            assert(!BufferPtr->NextBufferPtr);
            if (!_LastPtr) {
                _FirstPtr = _LastPtr = BufferPtr;
            } else {
                _LastPtr->NextBufferPtr = BufferPtr;
                _LastPtr = BufferPtr;
            }
            ++_TotalBufferCount;
        }
        X_INLINE size_t Push(const void * DataPtr, size_t DataSize) {
            if (!_LastPtr) {
                return 0;
            }
            return _LastPtr->Push(DataPtr, DataSize);
        }

        X_INLINE size_t GetSize () const { return _TotalBufferCount; }
        X_INLINE bool   IsEmpty () const { return !GetSize(); }
        X_INLINE xPacketBuffer * GetLast() const { return _LastPtr; }

    private:
        xPacketBuffer * _FirstPtr = nullptr;
        xPacketBuffer * _LastPtr = nullptr;
        size_t _TotalBufferCount = 0;
    };

}