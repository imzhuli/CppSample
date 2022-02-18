#include <zec_ext/IO/PacketData.hpp>
#include <zec/Byte.hpp>

ZEC_NS
{

    static_assert(std::is_standard_layout_v<xPacketHeader>);
    static_assert(std::is_standard_layout_v<xPacket>);
    static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
    static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);
    static_assert(sizeof(xPacketHeader) == offsetof(xPacket, Payload));

    ZEC_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketLength) {
        assert(PacketLength <= MaxPacketSize);
        return PacketLength | PacketMagicValue;
    }

    ZEC_STATIC_INLINE bool CheckPackageLength(uint32_t PacketLength) {
        return (PacketLength & PacketMagicMask) == PacketMagicValue
            && (PacketLength & PacketLengthMask) <= MaxPacketSize;
    }

    void xPacketHeader::Serialize(void * DestPtr) const
    {
        xStreamWriter S(DestPtr);
        S.W4L(MakeHeaderLength(PacketLength));
        S.W1L(PackageSequenceId);
        S.W1L(PackageSequenceTotalMax);
        S.W2L(CommandId);
        S.W8L(RequestId);
        S.W(TraceId, 16);
    }

    size32_t xPacketHeader::Deserialize(const void * SourcePtr)
    {
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

}
