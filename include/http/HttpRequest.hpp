#pragma once

#include <string>			// std::string
#include <map>				// std::map

class HTTPRequest
{

public:

	HTTPRequest(size_t maxBodySize);

	void parse(const std::string& rawBytes);

	bool isComplete() const;
	const std::string &getMethod() const;
	const std::string &getUri() const;
	const std::string &getVersion() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getBody() const;
	int errorCode() const;

private:

	// can't be default constructed, must provide max body size
	HTTPRequest();

	bool extractRequestLine(std::string &line, size_t &first, size_t &last);
	bool parseRequestLine();

	bool ExtractHeaders(std::string &headersPart);
	bool parseHeaders();

	void parseBody();
	void parseChunkedBody();
	void parseFixedBody();

	void setError(int code);

	std::string		rawBuffer_;
	size_t			bytesParsed_;

	// parsing state
	bool			complete_; 
	bool			headerParsed_;
	int				errorCode_;

	// request line
	std::string		method_;
	std::string		uri_;
	std::string		version_;
	bool			requestLineParsed_;

	// headers
	std::map<std::string, std::string> headers_;

	// body
	std::string		body_;
	size_t			contentLength_;   // parsed from Content-Length header, 0 if absent
	size_t			maxBodySize_;    // injected at construction, checked during buffering
	bool			chunked_;          // true if Transfer-Encoding: chunked

};
