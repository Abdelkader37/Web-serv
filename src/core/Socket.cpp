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

	// Set the close-on-exec flag to prevent file descriptor leaks in child processes
	fcntl(fd_, F_SETFD, FD_CLOEXEC);
	// Set the socket to non-blocking mode
	fcntl(fd_, F_SETFL, O_NONBLOCK);
}

Socket::Socket(int fd)
{
	if (fd == -1)
		throw std::runtime_error("Invalid socket file descriptor");

	// Set the close-on-exec flag to prevent file descriptor leaks in child processes
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	// Set the socket to non-blocking mode
	fcntl(fd, F_SETFL, O_NONBLOCK);

	fd_ = fd;
}

int Socket::get() const
{
	return fd_;
}

Socket::~Socket()
{
	close(fd_);
	fd_ = -1;
}
