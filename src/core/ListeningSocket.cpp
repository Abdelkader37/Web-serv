#include "core/ListeningSocket.hpp"	// ListeningSocket
#include "utils/NetworkUtils.hpp"

#include <sys/socket.h>				// setsockopt, bind, listen, accept, SO_REUSEADDR, SOL_SOCKET, SOMAXCONN
#include <netdb.h>					// getaddrinfo, freeaddrinfo, gai_strerror, addrinfo
#include <stdexcept>				// runtime_error


ListeningSocket::ListeningSocket(const std::string &bind_address, const std::string &bind_port) : Socket()
{
	// SO_REUSEADDR: allow port reuse after close
	int reuse = 1;
	if (setsockopt(get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
		throw std::runtime_error("Failed to set SO_REUSEADDR on socket");

	struct addrinfo *results;
	if (!NetworkUtils::resolve(bind_address, bind_port, results))
		throw std::runtime_error("Failed to resolve address '" + bind_address + ":" + bind_port + "'");

	if (bind(get(), results->ai_addr, sizeof(*results->ai_addr)) == -1)
		throw std::runtime_error("Failed to bind socket to address '" + bind_address + ":" + bind_port + "'");

	freeaddrinfo(results);

	// SOMAXCONN: maximum length allowed for the queue of pending connections
	if (listen(get(), SOMAXCONN) == -1)
		throw std::runtime_error("Failed to listen on socket");
}

int ListeningSocket::acceptConnection()
{
	return accept(get(), NULL, NULL);
}
