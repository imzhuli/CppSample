#include "./Protocol.hpp"

size_t ProtocolHeartbeat(void * DataPtr, size_t DataSize)
{
	assert(DataSize >= PacketHeaderSize);
	xPacketHeader Header;
	Header.CommandId = CmdId_KeepAlive;
	Header.PacketLength = PacketHeaderSize;
	Header.Serialize(DataPtr);
	return PacketHeaderSize;
}
