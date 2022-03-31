#pragma once
#include <zec/Common.hpp>
#include <zec/Byte.hpp>
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

    /***
        @brief Such class is a 'almost' direct mapping to stream data header.
        @note  Serialization uses Little-Endian
    */
    struct xPacketHeader final
    {
        static constexpr const size32_t Size = 2 * sizeof(uint64_t) + 16;

        xPacketLength            PacketLength = 0; // header size included, lower 24 bits as length, higher 8 bits as a magic check
        xPacketSequence          PackageSequenceId = 0; // the index of the packet in a full package, (this is no typo)
        xPacketSequence          PackageSequenceTotalMax = 0;
        xPacketCommandId         CommandId = 0;
        xPacketRequestId         RequestId = 0;
        ubyte                    TraceId[16] = {}; // allow uuid

        ZEC_API_MEMBER void      Serialize(void * DestPtr) const {
            xStreamWriter S(DestPtr);
            S.W4L(MakeHeaderLength(PacketLength));
            S.W1L(PackageSequenceId);
            S.W1L(PackageSequenceTotalMax);
            S.W2L(CommandId);
            S.W8L(RequestId);
            S.W(TraceId, 16);
        }

        ZEC_INLINE size32_t  Deserialize(const void * SourcePtr) {
            xStreamReader S(SourcePtr);
            PacketLength = S.R4L();
            if (!CheckPackageLength(PacketLength)) {
                return 0;
            }
            PacketLength &= PacketLengthMask;
            PackageSequenceId = S.R1L();
            PackageSequenceTotalMax = S.R1L();
            CommandId = S.R2L();
            RequestId = S.R8L();
            S.R(TraceId, 16);
            return PacketLength;
        }

        ZEC_STATIC_INLINE void PatchRequestId(void * PacketPtr, xPacketRequestId RequestId) {
            xStreamWriter S(PacketPtr);
            S.Skip(
                + 4 // PacketLength
                + 2 // SequenceId/SequenceTotal
                + 2); // CommandId
            S.W8L(RequestId);
        };

    private:
        ZEC_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketLength) {
            assert(PacketLength <= MaxPacketSize);
            return PacketLength | PacketMagicValue;
        }
        ZEC_STATIC_INLINE bool CheckPackageLength(uint32_t PacketLength) {
            return (PacketLength & PacketMagicMask) == PacketMagicValue
                && (PacketLength & PacketLengthMask) <= MaxPacketSize;
        }
    };

    struct xPacket
    {
        xPacketHeader Header;
        ubyte Payload[MaxPacketSize - xPacketHeader::Size];

        ZEC_STATIC_INLINE ubyte * GetPayload(void * PacketPtr) { return (ubyte *)PacketPtr + xPacketHeader::Size; }
        ZEC_STATIC_INLINE const ubyte * GetPayload(const void * PacketPtr) { return (const ubyte *)PacketPtr + xPacketHeader::Size; }
    };

}
