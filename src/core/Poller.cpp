#include "core/Poller.hpp"

#include <cstddef>
#include <poll.h>         // poll(), struct pollfd, POLLIN, POLLOUT

#define NOT_VALID(fd) ((fd) == -1 || (size_t)(fd) >= fd_to_index_.size() || fd_to_index_[(fd)] == -1)

Poller::Poller() : fd_to_index_(10240, -1) {}
Poller::~Poller() {}

void Poller::add(int fd, int events, Owner owner)
{
	if (fd == -1 || events == 0)
		return;

	if ((size_t)fd >= fd_to_index_.size())
		fd_to_index_.resize(fd + 1, -1);

	struct pollfd pfd;
	pfd.fd      = fd;
	pfd.events  = events;
	pfd.revents = 0;

	pfds_.push_back(pfd);
	owners_.push_back(owner);

	fd_to_index_[fd] = pfds_.size() - 1;
}

void Poller::mod(int fd, int events)
{
	if (NOT_VALID(fd))
		return;

	pfds_[fd_to_index_[fd]].events = events;
}

void Poller::remove(int fd)
{
	if (NOT_VALID(fd))
		return;

	const int idx     = fd_to_index_[fd];
	const int last_fd = pfds_.back().fd;

	pfds_[idx]    = pfds_.back();
	owners_[idx]  = owners_.back();

	pfds_.pop_back();
	owners_.pop_back();

	fd_to_index_[last_fd] = idx;
	fd_to_index_[fd]      = -1;
}

void Poller::add(Socket &socket, int events, Owner owner) { add(socket.get(), events, owner); }
void Poller::mod(Socket &socket, int events)              { mod(socket.get(), events); }
void Poller::remove(Socket &socket)                       { remove(socket.get()); }

void Poller::add(Pipe &pipe, int events, Owner owner)
{
	add(pipe.getReadFd(),  events & POLLIN,  owner);
	add(pipe.getWriteFd(), events & POLLOUT, owner);
}

// events must be a subset of what was add()ed originally
void Poller::mod(Pipe &pipe, int events)
{
	mod(pipe.getReadFd(),  events & POLLIN);
	mod(pipe.getWriteFd(), events & POLLOUT);
}

void Poller::remove(Pipe &pipe)
{
	remove(pipe.getReadFd());
	remove(pipe.getWriteFd());
}

void Poller::waitAndDispatch(int timeout_ms, void (*dispatcher)(Owner owner, int revents))
{

	int num_events = poll(pfds_.data(), pfds_.size(), timeout_ms);
	if (num_events <= 0)
		return;

	ready_.clear();
	for (size_t i = 0; i < pfds_.size(); ++i)
		if (pfds_[i].revents != 0)
			ready_.push_back(std::make_pair(owners_[i], pfds_[i].revents));

	for (size_t i = 0; i < ready_.size(); ++i)
		dispatcher(ready_[i].first, ready_[i].second);
}
