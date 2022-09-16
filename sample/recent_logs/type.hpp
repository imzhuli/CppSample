#pragma once
#include <zec/Common.hpp>
#include <zec/List.hpp>
#include <zec/String.hpp>
#include <zec/Util/Command.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/TcpConnectionEx.hpp>
#include <zec_ext/IO/TcpServer.hpp>
#include <iostream>
#include <string>

using namespace zec;
using namespace std;

struct xLogRecord : xListNode
{
	uint64_t    ServerTimeMS;
	uint64_t    LogTimestampMS;
	std::string LogContents;
};
using xLogRecordList = xList<xLogRecord>;

class xCheckHttpConnection
: public xTcpConnectionEx
{
public:
    std::string RequestLine;
	size_t      FirstLineEnd = 0;
    size_t      HeaderSize = 0;
    size_t      BodySizeChecked = false;
    size_t      BodySize = 0;
};

class xCheckHttpServer
: xTcpServer::iListener
, xTcpConnection::iListener
{
public:
    bool Init(xIoContext * IoContextPtr, const xNetAddress & NetAddress);
    void Clean();
	void SetLogRequestPath(const std::string & Path);
    void Shrink();

protected:
	void    OnNewConnection(xTcpServer * TcpServerPtr, xIoHandle NativeHandle) override;
    size_t  OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;
    void    OnPeerClose(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); };
    void    OnError(xTcpConnection * TcpConnectionPtr) override { Kill(TcpConnectionPtr); };
    bool    OnConnectionRequest(xCheckHttpConnection * TcpConnectionPtr);

    void Kill(xTcpConnection * TcpConnectionPtr) {
        KillList.GrabTail(*(xTcpConnectionEx*)TcpConnectionPtr);
    }

    void AddLogRecord(xLogRecord * Record) {
        if (RecordListLength >= MaxLogs) {
            delete RecordList.PopHead();
        }
        RecordList.AddTail(*Record);
    }

    void ClearLogRecordList() {
        while(auto RecordPtr = RecordList.PopHead()) {
            delete RecordPtr;
        }
        RecordListLength = 0;
    }

private:
    xTcpServer           TcpServer;
    xTcpConnectionExList TimeoutList;
    xTcpConnectionExList KillList;

    size_t               MaxLogs=200;
    size_t               RecordListLength = 0;
    xLogRecordList       RecordList;
	std::string          GetLogRequestPath;
	std::string          PostLogRequestPath;
};
