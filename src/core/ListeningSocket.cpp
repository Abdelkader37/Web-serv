#include "core/ListeningSocket.hpp"

#include <sys/socket.h>    // setsockopt, bind, listen, SOL_SOCKET, SO_REUSEADDR, SOMAXCONN
#include <netdb.h>         // getaddrinfo, freeaddrinfo, gai_strerror, addrinfo, AI_PASSIVE
#include <netinet/in.h>    // sockaddr_in, AF_INET

ListeningSocket::ListeningSocket(const std::string &bind_address, const std::string &bind_port) : Socket()
{
	// SO_REUSEADDR: allow port reuse after close
	int reuse = 1;
	if (setsockopt(get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
		throw std::runtime_error("Failed to set SO_REUSEADDR on socket");

	struct addrinfo hints = {};
	// AF_INET: IPv4
	hints.ai_family   = AF_INET;
	// SOCK_STREAM: TCP
	hints.ai_socktype = SOCK_STREAM;
	// AI_PASSIVE: wildcard (0.0.0.0) instead of loopback when node (first arg) is NULL
	hints.ai_flags    = AI_PASSIVE; 

	struct addrinfo *results;
	int resolve_status = getaddrinfo(bind_address.empty() ? NULL : bind_address.c_str(), bind_port.c_str(), &hints, &results);
	if (resolve_status != 0)
		throw std::runtime_error("Failed to resolve address '" + bind_address + ":" + bind_port + "': " + gai_strerror(resolve_status));

	struct sockaddr_in resolved_address = *(reinterpret_cast<struct sockaddr_in *>(results->ai_addr));

	freeaddrinfo(results);

	if (bind(get(), (reinterpret_cast<struct sockaddr *>(&resolved_address)), sizeof(resolved_address)) == -1)
		throw std::runtime_error("Failed to bind socket to address '" + bind_address + ":" + bind_port + "'");

	// SOMAXCONN: maximum length allowed for the queue of pending connections
	if (listen(get(), SOMAXCONN) == -1)
		throw std::runtime_error("Failed to listen on socket");
}

int ListeningSocket::acceptConnection()
{
	return accept(get(), NULL, NULL);
}
