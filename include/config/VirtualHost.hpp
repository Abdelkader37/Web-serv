#pragma once

#include "config/Route.hpp"			// Route

#include <string>					// std::string
#include <vector>					// std::vector
#include <utility>					// std::pair
#include <map>						// std::map

// for readability
typedef int statusCode;

class VirtualHost
{

public:

	VirtualHost();

	const std::vector<std::pair<std::string, std::string> >	&binds() const;
	const std::string										&name() const;
	const std::map<statusCode, std::string>					&errorPages() const;
	const std::vector<Route>								&routes() const;

private:

	std::vector<std::pair<std::string, std::string> >	binds_;
	std::map<statusCode, std::string>					errorPages_;
	std::vector<Route>									routes_;

};
