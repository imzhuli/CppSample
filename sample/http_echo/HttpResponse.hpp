#pragma once
#include "./Http.hpp"
#include <zec/Common.hpp>
#include <string>
#include <vector>

ZEC_NS
{

	class xHttpResponse
	{
	public:
		int32_t                 ResponseCode { 200 };
		std::string             ContentType;
		std::vector<xCookie>    CookieList;
		std::string             Body;

	public:
		ZEC_INLINE xHttpResponse() = default;
		ZEC_INLINE xHttpResponse(xHttpResponse &&) = default;
		ZEC_INLINE void SetServerError() { ResponseCode = 500; };
		ZEC_INLINE bool IsServerError() const { return ResponseCode == 500; };
	};

}