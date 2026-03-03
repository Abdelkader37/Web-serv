#include "core/Pipe.hpp"

#include <unistd.h>       // pipe(), close()
#include <stdexcept>      // std::runtime_error
#include <fcntl.h>       // fcntl(), F_SETFD, FD_CLOEXEC

Pipe::Pipe()
{
	int fds[2];
	if (pipe(fds) == -1)
		throw std::runtime_error("Failed to create pipe");

	// Set the close-on-exec flag to prevent file descriptor leaks in child processes
	fcntl(fds[0], F_SETFD, FD_CLOEXEC);
	fcntl(fds[1], F_SETFD, FD_CLOEXEC);

	readFd_ = fds[0];
	writeFd_ = fds[1];
}

Pipe::~Pipe()
{
	closeWriteFd();
	closeReadFd();
}

int Pipe::getReadFd() const
{
	return readFd_;
}

int Pipe::getWriteFd() const
{
	return writeFd_;
}

void Pipe::closeWriteFd()
{
	close(writeFd_);
	writeFd_ = -1;
}

void Pipe::closeReadFd()
{
	close(readFd_);
	readFd_ = -1;
}
