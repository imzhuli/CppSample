#pragma once
#include <xel/Common.hpp>
#include <xel/Byte.hpp>
#include <algorithm>
#include <cstring>

X_NS
{
    using xPacketCommandId = uint16_t;
    using xPacketLength    = uint32_t;
    using xPacketRequestId = uint64_t;
    using xPacketSequence  = uint8_t;

    static constexpr const size_t   PacketHeaderSize = 32u;
    static constexpr const uint32_t PacketMagicMask  = 0xFF'000000u;
    static constexpr const uint32_t PacketMagicValue = 0xCD'000000u;
    static constexpr const uint32_t PacketLengthMask = 0x00'FFFFFFu;

    static constexpr const size_t   InvalidPacketSize          = static_cast<size_t>(-1);
    static constexpr const size_t   MaxPacketSize              = 4096 & PacketLengthMask;
    static constexpr const size_t   MaxPacketPayloadSize       = MaxPacketSize - PacketHeaderSize;

    /***
        @brief Such class is a 'almost' direct mapping to stream data header.
        @note  Serialization uses Little-Endian
    */
    struct xPacketHeader final
    {
        static constexpr const size_t Size = 2 * sizeof(uint64_t) + 16;
        static constexpr const xPacketCommandId CmdId_InnernalRequest             = 0x00'00;
        static constexpr const xPacketRequestId InternalRequest_KeepAlive         = 0x00;
        static constexpr const xPacketRequestId InternalRequest_RequestKeepAlive  = static_cast<uint64_t>(-1);

        xPacketLength            PacketLength = 0; // header size included, lower 24 bits as length, higher 8 bits as a magic check
        xPacketCommandId         CommandId = 0;
        xPacketRequestId         RequestId = 0;
        ubyte                    TraceId[16] = {}; // allow uuid

        X_INLINE void Serialize(void * DestPtr) const {
            xStreamWriter S(DestPtr);
            S.W4L(MakeHeaderLength(PacketLength));
            S.W2L(CommandId);
            S.W8L(RequestId);
            S.W(TraceId, 16);
        }

        X_INLINE size32_t Deserialize(const void * SourcePtr) {
            xStreamReader S(SourcePtr);
            PacketLength = PickPackageLength(S.R4L());
            CommandId = S.R2L();
            RequestId = S.R8L();
            S.R(TraceId, 16);
            return PacketLength;
        }

        X_INLINE size32_t Deserialize(const void * SourcePtr, size_t PacketSizeLimit) {
            Deserialize(SourcePtr);
            return PacketLength <= PacketSizeLimit ? PacketLength : 0;
        }

        X_INLINE size_t GetPayloadSize() const {
            return PacketLength - PacketHeaderSize;
        }

        X_STATIC_INLINE void PatchRequestId(void * PacketPtr, xPacketRequestId RequestId) {
            xStreamWriter S(PacketPtr);
            S.Skip(
                + 4 // PacketLength
                + 2); // CommandId
            S.W8L(RequestId);
        };

        X_STATIC_INLINE size_t MakeKeepAlive(void * PackageHeaderBuffer) {
            xPacketHeader Header;
            Header.CommandId = CmdId_InnernalRequest;
            Header.RequestId = InternalRequest_KeepAlive;
            Header.PacketLength = PacketHeaderSize;
            Header.Serialize(PackageHeaderBuffer);
            return PacketHeaderSize;
        }

        X_STATIC_INLINE size_t MakeCheckKeepAlive(void * PackageHeaderBuffer) {
            xPacketHeader Header;
            Header.CommandId = CmdId_InnernalRequest;
            Header.RequestId = InternalRequest_RequestKeepAlive;
            Header.PacketLength = PacketHeaderSize;
            Header.Serialize(PackageHeaderBuffer);
            return PacketHeaderSize;
        }

        X_INLINE bool IsInternalRequest() const {
            return CommandId == CmdId_InnernalRequest;
        }

        X_INLINE bool IsKeepAlive() const {
            return IsInternalRequest() && RequestId == InternalRequest_KeepAlive;
        }

        X_INLINE bool IsRequestKeepAlive() const {
            return IsInternalRequest() && RequestId == InternalRequest_RequestKeepAlive;
        }

    private:
        X_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketLength) {
            assert(PacketLength <= PacketLengthMask);
            return PacketLength | PacketMagicValue;
        }
        X_STATIC_INLINE uint32_t PickPackageLength(uint32_t PacketLengthField) {
            uint32_t PacketLength = PacketLengthField ^ PacketMagicValue;
            return X_LIKELY(PacketLength <= PacketLengthMask) ? PacketLength : 0;
        }
    };

    struct xPacket
    {
        X_STATIC_INLINE ubyte * GetPayload(void * PacketPtr) { return (ubyte *)PacketPtr + xPacketHeader::Size; }
        X_STATIC_INLINE const ubyte * GetPayload(const void * PacketPtr) { return (const ubyte *)PacketPtr + xPacketHeader::Size; }
    };

}
