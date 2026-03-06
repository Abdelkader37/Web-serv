#include "core/Poller.hpp"
#include <poll.h>
#include <cassert>

Poller::Poller() : fd_to_index_(10240, -1) {}
Poller::~Poller() {}

void Poller::add(int fd, int events, Owner owner)
{
	if (fd == -1)
		return;
	if (fd >= (int)fd_to_index_.size())
		fd_to_index_.resize(fd + 1, -1);

	struct pollfd pfd;
	pfd.fd      = fd;
	pfd.events  = events;
	pfd.revents = 0;

	fd_to_index_[fd] = pfds_.size();

	pfds_.push_back(pfd);
	owners_.push_back(owner);
}

void Poller::mod(int fd, int events)
{
	if (fd == -1 || fd >= (int)fd_to_index_.size() || fd_to_index_[fd] == -1)
		return;

	pfds_[fd_to_index_[fd]].events = events;
}

void Poller::remove(int fd)
{
	if (fd == -1 || fd >= (int)fd_to_index_.size() || fd_to_index_[fd] == -1)
		return;

	pfds_[fd_to_index_[fd]]   = pfds_.back();
	owners_[fd_to_index_[fd]] = owners_.back();
	fd_to_index_[pfds_.back().fd] = fd_to_index_[fd];

	fd_to_index_[fd]      = -1;

	pfds_.pop_back();
	owners_.pop_back();
}

void Poller::add(Socket &socket, int events, Owner owner) { add(socket.get(), events, owner); }
void Poller::mod(Socket &socket, int events)              { mod(socket.get(), events); }
void Poller::remove(Socket &socket)                       { remove(socket.get()); }

void Poller::add(Pipe &pipe, int events, Owner owner)
{
	if (events & POLLIN)  add(pipe.getReadFd(),  POLLIN,  owner);
	if (events & POLLOUT) add(pipe.getWriteFd(), POLLOUT, owner);
}

void Poller::mod(Pipe &pipe, int events)
{
	if (events & POLLIN)  mod(pipe.getReadFd(),  POLLIN);
	if (events & POLLOUT) mod(pipe.getWriteFd(), POLLOUT);
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