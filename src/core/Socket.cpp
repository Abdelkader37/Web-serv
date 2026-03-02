#include "core/Socket.hpp"

#include <sys/socket.h>   // socket()
#include <netinet/in.h>   // AF_INET, SOCK_STREAM
#include <unistd.h>       // close()
#include <stdexcept>      // std::runtime_error

Socket::Socket()
{
	fd_ = socket(AF_INET, SOCK_STREAM, 0);

	if (fd_ == -1)
		throw std::runtime_error("Failed to create socket");
}

Socket::Socket(int fd)
{
	if (fd == -1)
		throw std::runtime_error("Invalid socket file descriptor");

	fd_ = fd;
}

int Socket::get() const
{
	return fd_;
}

Socket::~Socket()
{
	close(fd_);
}
