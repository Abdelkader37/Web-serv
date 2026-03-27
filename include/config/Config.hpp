#pragma once

#include "config/VirtualHost.hpp"	// VirtualHost

#include <string>					// std::string
#include <vector>					// std::vector

class Config
{

public:

	const std::vector<VirtualHost>	&parse(const std::string &filepath);

private:

	std::vector<VirtualHost> virtualHosts_;
};
