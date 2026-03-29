#pragma once

#include "utils/NetworkUtils.hpp"	// resolve

#include <string>					// string
#include <netdb.h>					// AI_PASSIVE, addrinfo, getaddrinfo
#include <sys/socket.h>				// AF_INET, SOCK_STREAM

bool NetworkUtils::resolve(const std::string &address, const std::string &port, struct addrinfo *&results)
{
	struct addrinfo hints = {};
	hints.ai_family		= AF_INET;		// IPv4
	hints.ai_socktype	= SOCK_STREAM;	// TCP
	hints.ai_flags		= AI_PASSIVE;	// wildcard (0.0.0.0) when address is empty

	return getaddrinfo(address.empty() ? NULL : address.c_str(), port.c_str(), &hints, &results) == 0;
}

bool NetworkUtils::resolve(const std::string &address, const std::string &port)
{
	struct addrinfo *results;
	bool ok = resolve(address, port, results);
	if (ok)
		freeaddrinfo(results);
	return ok;
}

bool NetworkUtils::resolve(const std::string &address_port)
{
	size_t colon = address_port.rfind(':');
	if (colon == std::string::npos)
		return false;
	return resolve(address_port.substr(0, colon), address_port.substr(colon + 1));
}
