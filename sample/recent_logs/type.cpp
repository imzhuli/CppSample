#include "./type.hpp"
#include <ctime>
#include <sstream>

static constexpr const size_t    MaxSafeStringLength = 4096;
static constexpr const uint64_t  CheckHttpRequestTimeoutMS = 5'000;
static const size_t              MaxHeadSize = 4096;
static const size_t              MaxPostBodySize = 3 * MaxSafeStringLength;
static const char                ContentLengthHint[] = "Content-Length: ";

static std::string MakeSafeString(const std::string &Source)
{
	if (Source.length() < MaxSafeStringLength) {
		return Source;
	}
	auto String32 = ToUtf32(Source);
	if (String32.length () >= MaxSafeStringLength) {
		String32 = String32.substr(0, MaxSafeStringLength);
		String32[MaxSafeStringLength - 5] = L' ';
		String32[MaxSafeStringLength - 4] = L'.';
		String32[MaxSafeStringLength - 3] = L'.';
		String32[MaxSafeStringLength - 2] = L'.';
		String32[MaxSafeStringLength - 1] = L' ';
	}
	return ToUtf8(String32);
}

bool xCheckHttpServer::Init(xIoContext * IoContextPtr, const xNetAddress & NetAddress)
{
    return TcpServer.Init(IoContextPtr, NetAddress, this);
}

void xCheckHttpServer::Clean()
{
	ClearLogRecordList();
    TcpServer.Clean();
}

void xCheckHttpServer::SetLogRequestPath(const std::string & Path)
{
	GetLogRequestPath = "GET " + Path;
	PostLogRequestPath = "POST " + Path;
	ClearLogRequestPath = GetLogRequestPath + "/clear";
}

void xCheckHttpServer::OnNewConnection(xTcpServer * TcpServerPtr, xIoHandle NativeHandle)
{
	uint64_t NowMS = GetMilliTimestamp();
    xCheckHttpConnection * ConnectionPtr = new (std::nothrow) xCheckHttpConnection();
    if (!ConnectionPtr) {
        return;
    }
    if (!ConnectionPtr->Init(NativeHandle, this)) {
        delete ConnectionPtr;
        return;
    }
    ConnectionPtr->ConnectionTimestampMS = NowMS;
    TimeoutList.AddTail(*ConnectionPtr);
}

size_t xCheckHttpServer::OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize)
{
    auto ConnectionPtr = (xCheckHttpConnection*)TcpConnectionPtr;
    auto & RequestLine = ConnectionPtr->RequestLine;

    RequestLine.append((const char *)DataPtr, DataSize);
	if (!ConnectionPtr->HeaderSize) {
		if (RequestLine.size() >= MaxHeadSize + MaxPostBodySize) {
			Kill(ConnectionPtr);
			return 0;
		}

		auto FirstLineEnd = RequestLine.find("\r\n");
		if (FirstLineEnd == RequestLine.npos) {
			return DataSize;
		}
		ConnectionPtr->FirstLineEnd = FirstLineEnd;

		auto RequestLineEnd = RequestLine.find("\r\n\r\n", FirstLineEnd);
		if (RequestLineEnd == RequestLine.npos) {
			return DataSize;
		}
		ConnectionPtr->HeaderSize = RequestLineEnd + 4;
	}
	if (!ConnectionPtr->BodySizeChecked) {
		auto ContentLengthIndex = RequestLine.find(ContentLengthHint);
		if (ContentLengthIndex != RequestLine.npos) {
			auto BodySize = (size_t)atoll(&RequestLine[ContentLengthIndex + SafeLength(ContentLengthHint)]);
			if (BodySize >= MaxPostBodySize) {
				ConnectionPtr->PostData("HTTP/1.1 502 InvalidData(2B)\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 70);
				ConnectionPtr->GracefulClose();
				return DataSize;
			}
			ConnectionPtr->BodySize = BodySize;
		}
		ConnectionPtr->BodySizeChecked = true;
	}

	if (RequestLine.length() != ConnectionPtr->HeaderSize + ConnectionPtr->BodySize) {
		return DataSize;
	}

	if (OnConnectionRequest(ConnectionPtr)) {
		ConnectionPtr->GracefulClose();
		return DataSize;
	}

	TcpConnectionPtr->PostData("HTTP/1.1 404 InvalidRequest(NF)\r\nContent-Type: application/octet-stream\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 113);
    ConnectionPtr->GracefulClose();
    return DataSize;
}

bool xCheckHttpServer::OnConnectionRequest(xCheckHttpConnection * TcpConnectionPtr)
{
	auto RequestLine = std::string_view{ TcpConnectionPtr->RequestLine.data(), TcpConnectionPtr->FirstLineEnd };
	if (RequestLine.find(GetLogRequestPath) == 0 && RequestLine[GetLogRequestPath.length()] == ' ') {
		std::stringstream ss;
		for (auto & R : RecordList) {
			std::tm brokenTime;
			ZecLocalTime(X2Ptr((time_t)(R.ServerTimeMS/1000)), &brokenTime);
			char TimeFormatBuffer[64];
			sprintf(TimeFormatBuffer, "%02d%02d%02d:%02d%02d%02d ",
				brokenTime.tm_year + 1900 - 2000, brokenTime.tm_mon + 1, brokenTime.tm_mday,
				brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
			);
			ss << TimeFormatBuffer << ": " << R.LogContents << endl;
		}
		auto s = ss.str();
		auto h = std::string("HTTP/1.1 200 OK\r\n") + "Content-Type: text/plain; charset=UTF-8\r\nConnection: close\r\n" + "Content-Length: " + std::to_string(s.length()) + "\r\n\r\n";

    	TcpConnectionPtr->PostData(h.data(), h.length());
		if (s.length()) {
    		TcpConnectionPtr->PostData(s.data(), s.length());
		}
		return true;
	}
	if (RequestLine.find(PostLogRequestPath) == 0 && RequestLine[PostLogRequestPath.length()] == ' ' && TcpConnectionPtr->BodySize) {
		xLogRecord * RecordPtr = new xLogRecord;
		RecordPtr->ServerTimeMS = GetMilliTimestamp();
		RecordPtr->LogContents = MakeSafeString(std::string{ TcpConnectionPtr->RequestLine.data() + TcpConnectionPtr->HeaderSize, TcpConnectionPtr->BodySize});
		AddLogRecord(RecordPtr);
    	TcpConnectionPtr->PostData("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 97);
		return true;
	}

	if (RequestLine.find(ClearLogRequestPath) == 0 && RequestLine[ClearLogRequestPath.length()] == ' ') {
		ClearLogRecordList();
    	TcpConnectionPtr->PostData("HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 97);
		return true;
	}
	TcpConnectionPtr->PostData("HTTP/1.1 404 InvalidRequest(NB)\r\nContent-Type: application/octet-stream\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", 113);
	return false;
}

void xCheckHttpServer::Shrink()
{
	uint64_t NowMS = GetMilliTimestamp();
    auto KillTimepoint = NowMS - CheckHttpRequestTimeoutMS;
    for (auto & Iter : TimeoutList) {
        auto & Connection = (xCheckHttpConnection&)Iter;
        if (Connection.ConnectionTimestampMS > KillTimepoint) {
            break;
        }
        Connection.Clean();
        delete &Connection;
    }

    for (auto & Iter : KillList) {
        auto & Connection = (xCheckHttpConnection&)Iter;
        Connection.Clean();
        delete &Connection;
    }
}
