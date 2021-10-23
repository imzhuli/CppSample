#pragma once
#include <zec/Common.hpp>
#include <unordered_map>
#include <string>
#include <string_view>
#include <fcgiapp.h>

ZEC_NS
{
	using xCookieMap = std::unordered_map<std::string, std::string>;

	ZEC_PRIVATE void HttpOk(const FCGX_Request & Request);
	ZEC_PRIVATE void HttpError(const FCGX_Request & Request, const char * ErrorMessage);
	ZEC_PRIVATE void HttpText(const FCGX_Request & Request, const std::string_view & Text, const xCookieMap & CookieMap = {});
	ZEC_PRIVATE void HttpHtml(const FCGX_Request & Request, const std::string_view & Html, const xCookieMap & CookieMap = {});
	ZEC_PRIVATE void HttpJson(const FCGX_Request & Request, const std::string_view & Json, const xCookieMap & CookieMap = {});

	using FcgiRequestProcessor = void (*)(const FCGX_Request &, void *);
	ZEC_PRIVATE void FcgiRequestLoop(FcgiRequestProcessor Processor, void * ContextPtr);

	// Utils:
	ZEC_PRIVATE xCookieMap ParseCookies(const std::string_view & CookieString);
}