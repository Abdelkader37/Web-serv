#pragma once

#include "config/Route.hpp"			// Route


#include <string>					// string
#include <vector>					// vector
#include <utility>					// pair
#include <map>						// map

// for readability
typedef int statusCode;



class VirtualHost
{

public:

	VirtualHost();

	void addBind(const std::string &address, const std::string &port);
	void setName(const std::string &name);
	


	void addRoute(Route &route);
	
	const std::vector<std::pair<std::string, std::string> >	&binds() const;
	const std::string										&name() const;
	const std::map<statusCode, std::string>					&errorPages() const;
	const std::vector<Route>								&routes() const;

private:

	std::vector<std::pair<std::string, std::string> >	binds_;
	std::string											name_;
	std::map<statusCode, std::string>					errorPages_;
	std::vector<Route>									routes_;

};
