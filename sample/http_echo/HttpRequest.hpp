#pragma once
#include <zec/Common.hpp>
#include <unordered_map>
#include <string>
#include <fcgiapp.h>

ZEC_NS
{

	class xHttpRequest
	{
	public:
		std::string Path;
		std::string Query;
		std::string Method;

		std::unordered_map<std::string, std::string> Headers;
		std::string ContentType;
		std::string Body;

	public:
		ZEC_INLINE xHttpRequest() = default;
		ZEC_INLINE xHttpRequest(const FCGX_Request & Request) { FromFcgi(Request); }
		ZEC_API_MEMBER void FromFcgi(const FCGX_Request & Request);
	};

}
