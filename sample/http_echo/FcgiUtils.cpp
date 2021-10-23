#include "FcgiUtils.hpp"
#include <zec/String.hpp>

ZEC_NS
{

	void HttpOk(const FCGX_Request & Request)
	{
		FCGX_PutS("Status: 200\r\n\r\n", Request.out);
	}

	void HttpError(const FCGX_Request & Request, const char * ErrorMessage)
	{
		FCGX_PutS("Status: 500\r\n", Request.out);
		FCGX_PutS("Content-Type: text/html\r\n", Request.out);
		FCGX_PutS("\r\n", Request.out);
		if (ErrorMessage) {
			FCGX_PutS(ErrorMessage, Request.out);
		}
	}

	void HttpText(const FCGX_Request & Request, const std::string_view & Text, const xCookieMap & CookieMap)
	{
		FCGX_PutS("Status: 200\r\n", Request.out);
		FCGX_PutS("Cache-Control: no-cache\r\n", Request.out);
		for (const auto & [Key, Value] : CookieMap) {
			FCGX_FPrintF(Request.out, "Set-Cookie: %s=%s\r\n", Key.c_str(), Value.c_str());
		}
		FCGX_PutS("Content-Type: text/plain; charset=utf-8\r\n", Request.out);
		FCGX_PutS("\r\n", Request.out);
		FCGX_PutStr(Text.data(), (int)Text.size(), Request.out);
	}

	void HttpHtml(const FCGX_Request & Request, const std::string_view & Html, const xCookieMap & CookieMap)
	{
		FCGX_PutS("Status: 200\r\n", Request.out);
		FCGX_PutS("Cache-Control: no-cache\r\n", Request.out);
		for (const auto & [Key, Value] : CookieMap) {
			FCGX_FPrintF(Request.out, "Set-Cookie: %s=%s\r\n", Key.c_str(), Value.c_str());
		}
		FCGX_PutS("Content-Type: text/html; charset=utf-8\r\n", Request.out);
		FCGX_PutS("\r\n", Request.out);
		FCGX_PutStr(Html.data(), (int)Html.size(), Request.out);
	}

	void HttpJson(const FCGX_Request & Request, const std::string_view & Json, const xCookieMap & CookieMap)
	{
		FCGX_PutS("Status: 200\r\n", Request.out);
		FCGX_PutS("Cache-Control: no-cache\r\n", Request.out);
		for (const auto & [Key, Value] : CookieMap) {
			FCGX_FPrintF(Request.out, "Set-Cookie: %s=%s\r\n", Key.c_str(), Value.c_str());
		}
		FCGX_PutS("Content-Type: application/json; charset=utf-8\r\n", Request.out);
		FCGX_PutS("\r\n", Request.out);
		FCGX_PutStr(Json.data(), (int)Json.size(), Request.out);
	}

	void FcgiRequestLoop(FcgiRequestProcessor Processor, void * ContextPtr)
	{
		FCGX_Request Request;
		FCGX_InitRequest(&Request, 0, FCGI_FAIL_ACCEPT_ON_INTR);
		while (FCGX_Accept_r(&Request) >= 0) {
			Processor(Request, ContextPtr);
		}
		FCGX_Finish_r(&Request);
	}

	std::unordered_map<std::string, std::string> ParseCookies(const std::string_view & CookieString)
	{
		std::unordered_map<std::string, std::string> CookieMap;
		std::vector<std::string> Segs = Split(CookieString, "; ");
		for (const auto & Seg : Segs) {
			if (Seg.empty()) {
				continue;
			}
			auto Index = Seg.find("=");
			if (Index == Seg.npos) {
				continue;
			}
			CookieMap.insert_or_assign(
				Seg.substr(0, Index),
				Seg.substr(Index + 1, Seg.npos)
			);
		}
		return CookieMap;
	}

}