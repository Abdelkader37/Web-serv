#include "core/Pipe.hpp"

#include <unistd.h>        // pipe(), close()
#include <stdexcept>       // std::runtime_error
#include <fcntl.h>         // fcntl(), F_SETFD, F_SETFL, FD_CLOEXEC, O_NONBLOCK


void Pipe::closeRead()
{
	close(readFd_);
	readFd_ = -1;
}
void Pipe::closeWrite()
{
	close(writeFd_);
	writeFd_ = -1;
}

Pipe::Pipe() : readFd_(-1), writeFd_(-1)
{
	int fds[2];
	if (pipe(fds) == -1)
		throw std::runtime_error("Failed to create pipe");

	// Set the close-on-exec flag to prevent file descriptor leaks in child processes
	// Also set the pipe to non-blocking mode
	if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) == -1 || fcntl(fds[1], F_SETFD, FD_CLOEXEC) == -1
		|| fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1 || fcntl(fds[1], F_SETFL, O_NONBLOCK) == -1)
	{
		closeRead();
		closeWrite();
		throw std::runtime_error("Failed to set FD_CLOEXEC and O_NONBLOCK flags on pipe file descriptors");
	}

	readFd_ = fds[0];
	writeFd_ = fds[1];
}
Pipe::~Pipe()
{
	close(readFd_);
	close(writeFd_);
}

int Pipe::getReadFd() const
{
	return readFd_;
}

int Pipe::getWriteFd() const
{
	return writeFd_;
}
