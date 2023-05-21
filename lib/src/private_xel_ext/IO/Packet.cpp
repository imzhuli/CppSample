#include <xel_ext/IO/Packet.hpp>
#include <xel/Byte.hpp>

X_NS
{

    static_assert(std::is_standard_layout_v<xPacketHeader>);
    static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
    static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);

}
