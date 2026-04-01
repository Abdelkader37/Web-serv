#pragma once




#include <string>					// string









namespace HttpStatus
{

enum Code
{
	// Used internally.
	NeedMoreData = 0, 

	/*####### 2xx - Successful #######*/
	OK        = 200,
	Created   = 201,
	NoContent = 204,

	/*####### 3xx - Redirection #######*/
	MovedPermanently  = 301,
	Found             = 302,
	SeeOther          = 303,
	TemporaryRedirect = 307,
	PermanentRedirect = 308,

	/*####### 4xx - Client Error #######*/
	BadRequest                  = 400,
	Forbidden                   = 403,
	NotFound                    = 404,
	MethodNotAllowed            = 405,
	ContentTooLarge             = 413,
	URITooLong                  = 414,
	IamATeapot                  = 418,
	RequestHeaderFieldsTooLarge = 431,

	/*####### 5xx - Server Error #######*/
	InternalServerError     = 500,
	NotImplemented          = 501,
	GatewayTimeout          = 504,
	HTTPVersionNotSupported = 505,
};

inline std::string reasonPhrase(int code)
{
	switch (code)
	{
	case 200: return "OK";
	case 201: return "Created";
	case 204: return "No Content";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 307: return "Temporary Redirect";
	case 308: return "Permanent Redirect";
	case 400: return "Bad Request";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 413: return "Content Too Large";
	case 414: return "URI Too Long";
	case 418: return "I'm a teapot";
	case 431: return "Request Header Fields Too Large";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 504: return "Gateway Timeout";
	case 505: return "HTTP Version Not Supported";
	default:  return std::string();
	}
}
// Not protected, validate input before use
inline Code toCode(const std::string &code)
{
	return static_cast<Code>((code[0] - '0') * 100 + (code[1] - '0') * 10 + (code[2] - '0'));
}

inline bool isRedirectCode(Code code)
{
	switch (code)
	{
	case 301: case 302: case 303: case 307: case 308:
		return true;
	default:
		return false;
	}
}
inline bool isRedirectCode(const std::string &code)
{
	return code == "301" || code == "302" || code == "303" || code == "307" || code == "308";
}

} // namespace HttpStatus
