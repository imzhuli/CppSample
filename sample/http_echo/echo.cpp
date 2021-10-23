#include <zec/Common.hpp>
#include <zec/View.hpp>
#include "FcgiUtils.hpp"
#include "HttpRequest.hpp"
#include <thread>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <csignal>
#include <iostream>
#include <string>
#include <vector>

using namespace zec;
using namespace std;

static constexpr size_t ThreadCount = 1;
static std::mutex FcgiOperationMutex;

void ProcessRequest(const FCGX_Request & Request, void * ContextPtr)
{
	std::vector<std::string> Headers;
	std::string ThreadId = std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));

	xHttpRequest FcgiRequest;
	FcgiRequest.FromFcgi(Request);

	// header
	FCGX_FPrintF(Request.out,
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n",
			ThreadId.c_str()
			);

	// respose
	FCGX_FPrintF(Request.out,
			"<title>FastCGI Echo!</title>"
			"<h1>FastCGI Echo! Query=(%s) : TID=(%s)</h1>",
			FcgiRequest.Query.c_str(),
			ThreadId.c_str()
			);

	FCGX_FPrintF(Request.out, "<div>Method: %s</div>", FcgiRequest.Method.c_str());
	for (const auto & [Key, Value] : FcgiRequest.Headers) {
		FCGX_FPrintF(Request.out, "<div>%s: %s</div>", Key.c_str(), Value.c_str());
	}
	if (!FcgiRequest.ContentType.empty()) {
		FCGX_FPrintF(Request.out, "<div>Content-Type: %s</div><div><p/></div>", FcgiRequest.ContentType.c_str());
	}
	FCGX_FPrintF(Request.out, "<div><p>%s</p></div>", FcgiRequest.Body.c_str());

	for (auto Line = Request.envp; *Line; ++Line) {
		FCGX_FPrintF(Request.out, "<div><li><span>%s</span></li></div>", *Line);
	}
};

int main(int, char **)
{
	FCGX_Init();
	std::thread ThreadList[ThreadCount];
	for (auto & ThreadObject : ThreadList) {
		ThreadObject = std::thread(FcgiRequestLoop, ProcessRequest, nullptr);
	}
	for (auto & ThreadObject : ThreadList) {
		ThreadObject.join();
	}

	return 0;
}