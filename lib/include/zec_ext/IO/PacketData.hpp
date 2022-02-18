#pragma once
#include <zec/Common.hpp>
#include <algorithm>
#include <cstring>

ZEC_NS
{

    static constexpr const size_t PacketHeaderSize           = 32;
    static constexpr const size_t PacketMagicMask            = 0xFF'000000;
    static constexpr const size_t PacketMagicValue           = 0xCD'000000;
    static constexpr const size_t PacketLengthMask           = 0x00'FFFFFF;
    static constexpr const size_t MaxPacketSize              = 4096 & PacketLengthMask;
    static constexpr const size_t MaxPacketPayloadSize       = MaxPacketSize - PacketHeaderSize;

    using xPacketCommandId     = uint16_t;
    using xPacketLength        = uint32_t;
    using xPacketSequence      = uint8_t;
    using xPacketRequestId     = uint64_t;

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
        ZEC_INLINE void RemoveFront() {
            assert(_FirstPtr);
            if (!(_FirstPtr = Steal(_FirstPtr->NextBufferPtr))) {
                _LastPtr = nullptr;
            }
            --TotalBufferCount;
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

    /***
        @brief Such class is a 'almost' direct mapping to stream data header.
        @note  Serialization uses Little-Endian
    */
    struct xPacketHeader final
    {
        xPacketLength            PacketLength = 0; // header size included, lower 24 bits as length, higher 8 bits as a magic check
        xPacketSequence          PackageSequenceId = 0; // the index of the packet in a full package, (this is no typo)
        xPacketSequence          PackageSequenceTotalMax = 0;
        xPacketCommandId         CommandId = 0;
        xPacketRequestId         RequestId = 0;
        ubyte                    TraceId[16] = {}; // allow uuid

        ZEC_API_MEMBER void      Serialize(void * DestPtr) const;
        ZEC_API_MEMBER size32_t  Deserialize(const void * SourcePtr);

        static constexpr const size32_t Size = 2 * sizeof(uint64_t) + 16;
    };

    struct xPacket
    {
        xPacketHeader Header;
        ubyte Payload[MaxPacketSize - xPacketHeader::Size];
    };

    ZEC_STATIC_INLINE ubyte * GetPayload(void * PacketPtr) { return (ubyte *)PacketPtr + xPacketHeader::Size; }
    ZEC_STATIC_INLINE const ubyte * GetPayload(const void * PacketPtr) { return (const ubyte *)PacketPtr + xPacketHeader::Size; }

}
