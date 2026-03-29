#pragma once

#include "http/HttpStatusCodes.hpp"	// HttpStatus::Code

#include <cstddef>					// size_t
#include <string>					// std::string
#include <vector>					// std::vector
#include <map>						// std::map

#define InstallDir "/usr/local/webserv"

#define HtmlDir InstallDir "/html"

class Route
{

public:

	Route();

	const std::string							&path() const;
	size_t										maxBodySize() const;
	const std::vector<std::string>				&methods() const;
	const std::string							&root() const;
	bool										redirected() const;
	const std::string							&redirectPage() const;
	HttpStatus::Code							redirectCode() const;
	bool										autoIndexed() const;
	const std::string							&indexFile() const;
	const std::map<std::string, std::string>	&cgis() const;
	const std::string							&uploadDir() const;

private:

	size_t								maxBodySize_;
	std::string							path_;
	std::vector<std::string>			methods_;
	std::string							root_;
	bool								redirect_;
	std::string							redirectPage_;
	HttpStatus::Code					redirectCode_;
	bool								autoindex_;
	std::string							indexFile_;
	std::map<std::string, std::string>	cgis_;
	std::string							uploadDir_;

};
