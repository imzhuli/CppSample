#include <zec_ext/IO/Packet.hpp>
#include <zec/Byte.hpp>

X_NS
{

    static_assert(std::is_standard_layout_v<xPacketHeader>);
    static_assert(std::is_standard_layout_v<xPacket>);
    static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
    static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);
    static_assert(sizeof(xPacketHeader) == offsetof(xPacket, Payload));

}
