#include "HttpRequest.hpp"
#include <cstring>
#include <cctype>

ZEC_NS
{


	void xHttpRequest::FromFcgi(const FCGX_Request & Request)
	{
		Path.clear();
		Query.clear();
		Headers.clear();
		ContentType.clear();
		Body.clear();
		Method.clear();

		const char * PathPtr = FCGX_GetParam("SCRIPT_NAME", Request.envp);
		if (PathPtr) {
			Path = PathPtr;
		}
		const char * QueryPtr = FCGX_GetParam("QUERY_STRING", Request.envp);
		if (QueryPtr) {
			Query = QueryPtr;
		}
		const char * MethodPtr = FCGX_GetParam("REQUEST_METHOD", Request.envp);
		if (MethodPtr) {
			Method = MethodPtr;
		}

		const char * ContentTypePtr = FCGX_GetParam("CONTENT_TYPE", Request.envp);
		if (ContentTypePtr) {
			ContentType = ContentTypePtr;
		}

		auto LenPtr = FCGX_GetParam("CONTENT_LENGTH", Request.envp);
		if (LenPtr) {
			int Len = (int)std::atoll(LenPtr);
			Body.resize(Len);
			FCGX_GetStr(Body.data(), Len, Request.in);
		}

		for(auto Line = Request.envp; *Line; ++Line) {
			if (strncmp(*Line, "HTTP_", 5)) {
				continue;
			}

			bool Cap = true;
			auto Name = *Line + 5;
			auto Curr = Name;
			std::string HName;
			std::string HValue;

			while(*Curr != '=') {
				char Char = *Curr++;
				assert(Char);

				if (Char == '_') {
					HName.push_back('-');
					Cap = true;
				}
				else {
					if (Cap) {
						HName.append(1, (char)std::toupper(Char));
						Cap = false;
					}
					else {
						HName.append(1, (char)std::tolower(Char));
					}
				}
			}
			Headers.insert(std::make_pair(std::move(HName), std::string(++Curr)));
		}

	}

}