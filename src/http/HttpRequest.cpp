#include "http/HTTPRequest.hpp"		// HTTPRequest class definition
#include "http/HttpStatusCodes.hpp"	// BadRequest, URITooLong, RequestHeaderFieldsTooLarge, HTTPVersionNotSupported, NotImplemented
#include "utils/StringUtils.hpp"	// toLower, trim
#include <cstddef>
#include <string>					// std::string, find, substr, rfind, size, empty
#include <cstdlib>					// std::strtoul
#include <algorithm>				// std::count

#define MAX_REQUEST_LINE_SIZE 8192 // 8KB
#define MAX_URI_SIZE          (MAX_REQUEST_LINE_SIZE - 15) // method(min 3 chars) + space + version(8 chars) + space + CRLF(2 chars)
#define MAX_HEADER_SIZE       16384 // 16KB

using namespace HttpStatus;
using namespace StringUtils;

HTTPRequest::HTTPRequest(size_t maxBodySize) : complete_(false), headerParsed_(false), requestLineParsed_(false), errorCode_(0), maxBodySize_(maxBodySize), bytesParsed_(0), contentLength_(0), chunked_(false)
{
	rawBuffer_.reserve(MAX_REQUEST_LINE_SIZE + MAX_HEADER_SIZE + maxBodySize_);
	method_.reserve(4);
	uri_.reserve(MAX_URI_SIZE);
	version_.reserve(8);
}

bool HTTPRequest::isComplete() const { return complete_; }
const std::string &HTTPRequest::getMethod() const { return method_; }
const std::string &HTTPRequest::getUri() const { return uri_; }
const std::string &HTTPRequest::getVersion() const { return version_; }
const std::map<std::string, std::string> &HTTPRequest::getHeaders() const { return headers_; }
const std::string &HTTPRequest::getBody() const { return body_; }
int HTTPRequest::errorCode() const { return errorCode_; }






static bool isValidMethodToken(const std::string &method)
{
	static const std::string tchars = "!#$%&'*+-.^_`|~";
	if (method.empty())
		return false;
	for (size_t i = 0; i < method.size(); i++)
	{
		unsigned char c = method[i];
		if (!std::isalnum(c) && tchars.find(c) == std::string::npos)
			return false;
	}
	return true;
}

static bool isValidMethod(const std::string &method)
{
	return method == "GET" || method == "POST" || method == "DELETE";
}

static int isValidVersion(const std::string &version, bool complete)
{
	if (version == "HTTP/1.0" || version == "HTTP/1.1")
		return 0;
	if (version.size() > 16)
		return version.find('.') != std::string::npos ? HTTPVersionNotSupported : BadRequest;
	static const std::string prefix = "HTTP/";
	if (version.size() < prefix.size())
		return prefix.substr(0, version.size()) == version ? (complete ? BadRequest : 0) : BadRequest;
	if (version.substr(0, 5) != "HTTP/")
		return BadRequest;
	if (!complete)
		return 0; // valid prefix so far, wait for more
	size_t dot = version.find('.', 5);
	if (dot == std::string::npos)
		return BadRequest;
	if (!isAllDigits(version, 5, dot) || !isAllDigits(version, dot + 1, version.size()))
		return BadRequest;
	return HTTPVersionNotSupported;
}

static int isValidRequestLine(const std::string &line, size_t &first, size_t &last, bool complete)
{
	size_t spaceCount = std::count(line.begin(), line.end(), ' ');
	if (spaceCount == 0 || spaceCount > 2)
		return BadRequest;
	first = line.find(' ');
	std::string method = line.substr(0, first);
	if (!isValidMethodToken(method))
		return BadRequest;
	if (!isValidMethod(method))
		return NotImplemented;
	last = line.rfind(' ');
	if (last - first - 1 > MAX_URI_SIZE)
		return URITooLong;
	if (spaceCount == 1)
		return complete ? BadRequest : 0; // soft overflow — valid method, URI within limit, no version, wait to see if space and version come in next chunk, if not, uri will hit the limit and return URITooLong
	return isValidVersion(line.substr(last + 1), complete);;
}

bool HTTPRequest::extractRequestLine(std::string &line, size_t &first, size_t &last)
{
	size_t lineEnd = rawBuffer_.find("\r\n");
	if (lineEnd == std::string::npos)
	{
		if (rawBuffer_.size() > MAX_REQUEST_LINE_SIZE)
			errorCode_ = isValidRequestLine(rawBuffer_, first, last, false);
		return false; // incomplete data
	}
	line = rawBuffer_.substr(0, lineEnd);
	errorCode_ = isValidRequestLine(line, first, last, true);
	if (errorCode_)
		return false;
	bytesParsed_ = lineEnd + 2;
	return true;
}

bool HTTPRequest::parseRequestLine()
{
	if (requestLineParsed_)
		return true;
	std::string line;
	size_t first = 0, last = 0;
	if (!extractRequestLine(line, first, last))
		return false; // incomplete data or invalid
	method_  = line.substr(0, first);
	uri_     = line.substr(first + 1, last - first - 1);
	version_ = line.substr(last + 1);
	requestLineParsed_ = true;
	return true;
}






bool HTTPRequest::ExtractHeaders(std::string &headersPart)
{
	size_t headersEnd = rawBuffer_.find("\r\n\r\n", bytesParsed_);
	if (headersEnd == std::string::npos)
	{
		if (rawBuffer_.size() - bytesParsed_ > MAX_HEADER_SIZE)
			errorCode_ = RequestHeaderFieldsTooLarge;
		return false; // incomplete data
	}
	headersPart = rawBuffer_.substr(bytesParsed_, headersEnd - bytesParsed_);
	bytesParsed_ = headersEnd + 4;
	return true;
}

static bool parseChunked(const std::map<std::string, std::string> &headers, const std::string &version, bool &chunked)
{
	std::map<std::string, std::string>::const_iterator te = headers.find("transfer-encoding");
	chunked = (te != headers.end() && toLower(te->second) == "chunked");
	return !(chunked && version == "HTTP/1.0");
}

static bool parseContentLength(const std::map<std::string, std::string> &headers, size_t &contentLength)
{
	std::map<std::string, std::string>::const_iterator cl = headers.find("content-length");
	if (cl == headers.end())
		return true;
	if (cl->second.empty() || !std::isdigit((unsigned char)cl->second[0]))
		return false;
	char *end;
	contentLength = std::strtoul(cl->second.c_str(), &end, 10);
	return *end == '\0';
}

static bool isValidHost(const std::map<std::string, std::string> &headers, const std::string &version)
{
	return !(version == "HTTP/1.1" && headers.find("host") == headers.end());
}

static int areValidHeaders(const std::map<std::string, std::string> &headers, const std::string &version, size_t &contentLength, bool &chunked)
{
	if (!parseChunked(headers, version, chunked))
		return (BadRequest);
	if (!chunked && !parseContentLength(headers, contentLength))
		return BadRequest;
	if (!isValidHost(headers, version))
		return BadRequest;
	return 0;
}

bool HTTPRequest::parseHeaders()
{
	if (headerParsed_)
		return true;
	std::string headersPart;
	if (!ExtractHeaders(headersPart))
		return false; // incomplete data
	size_t pos = 0;
	while (pos < headersPart.size())
	{
		size_t lineEnd = headersPart.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			break;
		std::string line = headersPart.substr(pos, lineEnd - pos);
		pos = lineEnd + 2;
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
		{
			errorCode_ = BadRequest;
			return false;
		}
		std::string name  = toLower(trim(line.substr(0, colonPos)));
		std::string value = trim(line.substr(colonPos + 1));
		headers_[name] = value;
	}
	errorCode_ = areValidHeaders(headers_, version_, contentLength_, chunked_);
	if (errorCode_)
		return false;
	if (!chunked_)
	{
		if (contentLength_ > maxBodySize_)
		{
			errorCode_ = ContentTooLarge;
			return false;
		}
		body_.reserve(contentLength_);
	}
	return true;
}






void HTTPRequest::parse(const std::string &rawBytes)
{
	if (complete_ || errorCode_)
		return;
	if (rawBuffer_.size() + rawBytes.size() > MAX_REQUEST_LINE_SIZE + MAX_HEADER_SIZE + maxBodySize_)
	{
		errorCode_ = ContentTooLarge;
		return;
	}
	rawBuffer_ += rawBytes;
	if (!headerParsed_)
	{
		if (!parseRequestLine())
			return;
		if (!parseHeaders())
			return;
		headerParsed_ = true;
	}
	parseBody();
}
