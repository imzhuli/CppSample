#pragma once
#include <zec/Common.hpp>
using namespace zec::common;

#include <zec/Byte.hpp>
using zec::xStreamReader;
using zec::xStreamWriter;

#include <zec/String.hpp>
using zec::StrToHex;
using zec::HexToStr;
using zec::HexShow;

#include <zec/List.hpp>
using zec::xListNode;
using zec::xList;

#include <zec/Util/IniReader.hpp>
using zec::xIniReader;

#include <zec/Memory.hpp>
using zec::xRetainable;

#include <zec/Util/MemoryPool.hpp>
using zec::xMemoryPoolOptions;
using zec::xMemoryPool;

#include <zec/Util/IndexedStorage.hpp>
using zec::xIndexId;
using zec::xIndexIdPool;
using zec::xIndexedStorage;

#include <zec/Util/Command.hpp>
using zec::xCommandLine;

#include <zec/Util/Logger.hpp>
using zec::eLogLevel;
using zec::xLogger;
using zec::NonLoggerPtr;
using zec::xSimpleLogger;

#include <zec/Util/Chrono.hpp>
using zec::xSteadyxTimePoint;
using zec::xTimer;
using namespace std::chrono_literals;

#include <zec/Util/Thread.hpp>
using zec::xSpinlock;
using zec::xEvent;

#include <zec_ext/Utility/Uuid.hpp>
using zec::xUuid;

#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/NetBase.hpp>
#include <zec_ext/IO/Packet.hpp>
#include <zec_ext/IO/TcpConnection.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <zec_ext/IO/WebSocket.hpp>
using zec::xIoHandle;
using zec::xIoContext;
using zec::xNetAddress;
using zec::xTcpConnection;
using zec::xTcpServer;
using zec::xWebSocketSession;

using zec::xPacketCommandId;
using zec::xPacketLength;
using zec::xPacketSequence;
using zec::xPacketRequestId;
using zec::xPacketHeader;
using zec::xPacket;

using zec::PacketHeaderSize;
using zec::PacketMagicMask;
using zec::PacketMagicValue;
using zec::PacketLengthMask;
using zec::MaxPacketSize;
using zec::MaxPacketPayloadSize;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <tuple>
#include <string>
#include <atomic>

#include <cstring>
#include <cstdlib>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

/*************
 * Protocol
 * ***********/
static constexpr const size_t MaxDeviceIdLength              = 127;
static constexpr const size_t MaxDeviceIdBufferSize          = MaxDeviceIdLength + 1;
static constexpr const size_t MaxAccountNameLength           = 127;
static constexpr const size_t MaxAccountNameBufferSize       = MaxAccountNameLength + 1;
static constexpr const size_t MaxAccountPasswordLength       = 127;
static constexpr const size_t MaxAccountPasswordBufferSize   = MaxAccountPasswordLength + 1;
static constexpr const size_t MaxSessionKeyLength            = 127;
static constexpr const size_t MaxSessionKeyBufferSize        = MaxSessionKeyLength + 1;

using xAuditDeviceStatus = uint8_t;
static constexpr const xAuditDeviceStatus AuditDeviceNotReady = 0;
static constexpr const xAuditDeviceStatus AuditDeviceOnline = 1;
static constexpr const xAuditDeviceStatus AuditDeviceOfflineTimeout = 2;
static constexpr const xAuditDeviceStatus AuditDeviceOfflinePeerClose = 3;
static constexpr const xAuditDeviceStatus AuditDeviceHeartbeat = 4;

using RequestObserverFlags = uint16_t;
static constexpr const RequestObserverFlags  RequestObserverNormal = 0;
static constexpr const RequestObserverFlags  RequestObserverListnerOnly = 1;

/*************
 * Utilities
 *************/
extern xNetAddress ParseIpAddress(const std::string & AddressStr);
