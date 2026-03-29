#pragma once

#include <string>					// std::string
#include <netdb.h>					// addrinfo

namespace NetworkUtils
{
	bool resolve(const std::string &address, const std::string &port, struct addrinfo *&results);
	bool resolve(const std::string &address, const std::string &port);
	bool resolve(const std::string &address_port);
}
