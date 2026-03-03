#include "core/ListeningSocket.hpp"

#include <stdexcept>       // std::runtime_error
#include <cstring>         // std::memset, std::memcpy
#include <sys/socket.h>     // setsockopt, bind, listen, SOL_SOCKET, SO_REUSEADDR, SOMAXCONN
#include <netdb.h>         // getaddrinfo, freeaddrinfo, gai_strerror, addrinfo, AI_PASSIVE
#include <netinet/in.h>    // sockaddr_in, AF_INET, INADDR_ANY, htons
#include <fcntl.h>         // fcntl, F_SETFL, O_NONBLOCK

ListeningSocket::ListeningSocket(const std::string &iface, int port) : Socket()
{
	int opt = 1;
	if (setsockopt(get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("setsockopt failed");

	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (iface.empty())
		hints.ai_flags = AI_PASSIVE;

	int ret = getaddrinfo(iface.empty() ? NULL : iface.c_str(), NULL, &hints, &res);
	if (ret != 0)
		throw std::runtime_error("getaddrinfo failed: " + std::string(gai_strerror(ret)));

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	std::memcpy(&addr, res->ai_addr, res->ai_addrlen);
	addr.sin_port = htons(port);
	freeaddrinfo(res);

	if (bind(get(), (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("Failed to bind socket");

	if (listen(get(), SOMAXCONN) == -1)
		throw std::runtime_error("Failed to listen on socket");

	if (fcntl(get(), F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode on listening socket");
}