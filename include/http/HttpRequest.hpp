#pragma once

#include <string>					// std::string
#include <map>						// std::map

#define MAX_REQUEST_LINE_SIZE 8192  // 8KB
#define MAX_HEADER_SIZE       16384 // 16KB

class HttpRequest
{

public:

	HttpRequest(size_t maxBodySize);

	void parse(const std::string &rawBytes);

	void reset();

	bool										complete() const;
	const std::string							&method() const;
	const std::string							&uri() const;
	const std::string							&version() const;
	const std::map<std::string, std::string>	&headers() const;
	const std::string							&body() const;
	int											errorCode() const;

private:

	// can't be default constructed, must provide max body size
	HttpRequest();

	// request line
	bool parseRequestLine();

	// headers
	bool parseHeaders();

	// body
	void parseBody();

	void cleanBuffer();

	// state
	bool	complete_;
	bool	headerParsed_;
	bool	requestLineParsed_;
	int		errorCode_;
	size_t	maxBodySize_;
	size_t	bytesParsed_;
	size_t	contentLength_;
	bool	chunked_;

	// data
	std::string							rawBuffer_;
	std::string							method_;
	std::string							uri_;
	std::string							version_;
	std::map<std::string, std::string>	headers_;
	std::string							body_;

};
