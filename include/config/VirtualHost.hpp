#pragma once

#include "config/Route.hpp"			// Route


#include <string>					// string
#include <vector>					// vector
#include <utility>					// pair
#include <map>						// map






class VirtualHost
{

public:

	VirtualHost();
	VirtualHost(const std::string &name);

	void addBind(const std::string &address, const std::string &port);
	void addErrorPage(const std::string &code, const std::string &page);
	void setName(const std::string &name);
	


	void addRoute(Route &route);
	
	const std::vector<std::pair<std::string, std::string> >	&binds() const;
	const std::string										&name() const;
	const std::map<HttpStatus::Code, std::string>			&errorPages() const;
	const std::vector<Route>								&routes() const;

private:

	std::vector<std::pair<std::string, std::string> >	binds_;
	std::string											name_;
	std::map<HttpStatus::Code, std::string>				errorPages_;
	std::vector<Route>									routes_;

};
