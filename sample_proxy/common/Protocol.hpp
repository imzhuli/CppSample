#pragma once
#include "../base/Common.hpp"
#include <string>

static constexpr size16_t MaxCommandNumber = 0xFFFFU;

static constexpr const xPacketCommandId CmdId_KeepAlive                         = 0x00;

static constexpr const xPacketCommandId CmdId_TerminalChallenge                 = 0x01;
static constexpr const xPacketCommandId CmdId_TerminalChallengeResp             = 0x02;

static constexpr const xPacketCommandId CmdId_TerminalChallengeGeo              = 0x03;
static constexpr const xPacketCommandId CmdId_TerminalChallengeGeoResp          = 0x04;

static constexpr const xPacketCommandId CmdId_TerminalNewConnection             = 0x05;
static constexpr const xPacketCommandId CmdId_TerminalCloseConnection           = 0x06;
static constexpr const xPacketCommandId CmdId_TerminalConnectionPayload         = 0x07;
static constexpr const xPacketCommandId CmdId_TerminalCheckKey                  = 0x08;
static constexpr const xPacketCommandId CmdId_TerminalNewHostConnection         = 0x09;
static constexpr const xPacketCommandId CmdId_TerminalConnectionEstablished     = 0x0A;
static constexpr const xPacketCommandId CmdId_TerminalBanVersion                = 0x0B;

static constexpr const xPacketCommandId CmdId_RelayServerInfo                   = 0x00'10;
// static constexpr const xPacketCommandId CmdId_UpdateDeviceInfo               = 0x00'11; // deprecated
// static constexpr const xPacketCommandId CmdId_RemoveDeviceInfo               = 0x00'12; // deprecated
// static constexpr const xPacketCommandId CmdId_QueryDeviceByRegion            = 0x00'13; // deprecated
// static constexpr const xPacketCommandId CmdId_QueryDeviceByRegionResp        = 0x00'14; // deprecated
// static constexpr const xPacketCommandId CmdId_QueryRegionTotal               = 0x00'15; // deprecated
// static constexpr const xPacketCommandId CmdId_QueryRegionTotalResp           = 0x00'16; // deprecated

/* server checks */
static constexpr const xPacketCommandId CmdId_ServerCheck                       = 0x00'FF;

/* events */
static constexpr const xPacketCommandId CmdId_EventDeviceStatusChange           = 0x01'00;
static constexpr const xPacketCommandId CmdId_EventAccountUsage                 = 0x01'01;

/* proxy exchange */
static constexpr const xPacketCommandId CmdId_ProxyConnectionAuth               = 0x02'00;
static constexpr const xPacketCommandId CmdId_ProxyConnectionAuthResp           = 0x02'01;
static constexpr const xPacketCommandId CmdId_ProxyNewConnection                = 0x02'02;
static constexpr const xPacketCommandId CmdId_ProxyNewHostConnection            = 0x02'03;
static constexpr const xPacketCommandId CmdId_ProxyCloseConnection              = 0x02'04;
static constexpr const xPacketCommandId CmdId_ProxyConnectionPrepared           = 0x02'05;
static constexpr const xPacketCommandId CmdId_ProxyConnectionEstablished        = 0x02'06;
static constexpr const xPacketCommandId CmdId_ProxyConnectionPayload            = 0x02'07;

/* audit */
static constexpr const xPacketCommandId CmdId_AuditReport                       = 0x0F'FD;

/* dispatcher */
static constexpr const xPacketCommandId CmdId_RegisterServer                    = 0xFF'FE;
static constexpr const xPacketCommandId CmdId_ErrorReport                       = 0xFF'FF;

extern size_t ProtocolHeartbeat(void * DataPtr, size_t DataSize);
