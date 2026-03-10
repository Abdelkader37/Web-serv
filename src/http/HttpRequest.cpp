#include "http/HTTPRequest.hpp"		// HTTPRequest class definition
#include "http/HttpStatusCodes.hpp"	// BadRequest, URITooLong, RequestHeaderFieldsTooLarge, HTTPVersionNotSupported
#include "utils/StringUtils.hpp"	// toLower, trim
#include <string>					// std::string, find, substr, rfind, size, empty

#define MAX_REQUEST_LINE_SIZE 8192
#define MAX_HEADER_SIZE       16384
#define MAX_URI_SIZE          MAX_REQUEST_LINE_SIZE - 16 // method(max 4 chars) + space + version(max 8 chars) + space + CRLF(2 chars)

using namespace HttpStatus;
using namespace StringUtils;

HTTPRequest::HTTPRequest(size_t maxBodySize) : complete_(false), headerParsed_(false), errorCode_(0), maxBodySize_(maxBodySize), bytesParsed_(0) {}

bool HTTPRequest::isComplete() const { return complete_; }
bool HTTPRequest::isChunked() const { return chunked_; }
const std::string &HTTPRequest::getMethod() const { return method_; }
const std::string &HTTPRequest::getUri() const { return uri_; }
const std::string &HTTPRequest::getVersion() const { return version_; }
const std::map<std::string, std::string> &HTTPRequest::getHeaders() const { return headers_; }
const std::string &HTTPRequest::getBody() const { return body_; }
int HTTPRequest::errorCode() const { return errorCode_; }

static bool isValidMethod(const std::string &method) { return method == "GET" || method == "POST" || method == "DELETE"; }

static int diagnoseRequestLineError(const std::string &line)
{
	size_t first = line.find(' ');
	size_t last  = line.rfind(' ');
	if (first == std::string::npos || first == last)
		return BadRequest;
	if (!isValidMethod(line.substr(0, first)))
		return NotImplemented;
	if (last - first - 1 > MAX_URI_SIZE)
		return URITooLong;
	std::string version = line.substr(last + 1);
	if (version == "HTTP/1.0" || version == "HTTP/1.1")
		return 0; // valid
	if (version.size() == 8 && version.substr(0, 5) == "HTTP/" 
		&& std::isdigit((unsigned char)version[5]) 
		&& version[6] == '.' 
		&& std::isdigit((unsigned char)version[7]))
		return HTTPVersionNotSupported;
	return BadRequest;
}

bool HTTPRequest::tryExtractRequestLine(std::string &line)
{
	size_t lineEnd = rawBuffer_.find("\r\n", bytesParsed_);
	if (lineEnd == std::string::npos)
	{
		if (rawBuffer_.size() - bytesParsed_ > MAX_REQUEST_LINE_SIZE)
			errorCode_ = diagnoseRequestLineError(rawBuffer_.substr(bytesParsed_));
		return false;
	}
	if (lineEnd - bytesParsed_ > MAX_REQUEST_LINE_SIZE)
	{
		errorCode_ = diagnoseRequestLineError(rawBuffer_.substr(bytesParsed_, lineEnd - bytesParsed_));
		return false;
	}
	line = rawBuffer_.substr(bytesParsed_, lineEnd - bytesParsed_);
	bytesParsed_ = lineEnd + 2;
	return true;
}

bool HTTPRequest::tryParseRequestLine()
{
	if (requestLineParsed_)
		return true;
	std::string line;
	if (!tryExtractRequestLine(line))
		return false; // incomplete data
	size_t first = line.find(' ');
	size_t last  = line.rfind(' ');
	if (first == std::string::npos || first == last)
	{
		errorCode_ = BadRequest;
		return false;
	}
	method_  = line.substr(0, first);
	uri_     = line.substr(first + 1, last - first - 1);
	version_ = line.substr(last + 1);
	errorCode_ = diagnoseRequestLineError(line);
	if (errorCode_)
		return false;
	requestLineParsed_ = true;
	return true;
}

void HTTPRequest::parse(const std::string &rawBytes)
{
	if (complete_ || errorCode_)
		return;
	rawBuffer_ += rawBytes;
	if (!headerParsed_)
	{
		if (!tryParseRequestLine())
			return;
		if (!tryParseHeaders())
			return;
		headerParsed_ = true;
	}
	parseBody();
}
