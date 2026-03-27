#pragma once

#include "http/HttpStatusCodes.hpp"	// HttpStatus::Code

#include <cstddef>
#include <string>					// std::string
#include <vector>					// std::vector
#include <map>						// std::map

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
	bool										autoindexed() const;
	const std::string							&indexFile() const;
	const std::map<std::string, std::string>	&cgiExt() const;
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
	std::map<std::string, std::string>	cgiExt_;
	std::string							uploadDir_;

};