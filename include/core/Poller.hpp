#pragma once

#include "Socket.hpp"      // Socket
#include "Pipe.hpp"        // Pipe
#include <vector>          // std::vector
#include <poll.h>          // struct pollfd
#include <utility>         // std::pair

// for readability purpose
typedef void* Owner;

class Poller
{

public:

	Poller();
	~Poller();

	// owner: caller context passed back in dispatcher, avoids fd-to-object lookup
	void add(Socket &socket, int events, Owner owner);
	void add(Pipe &pipe, int events, Owner owner);

	void mod(Socket &socket, int events);
	void mod(Pipe &pipe, int events);

	void remove(Socket &socket);
	void remove(Pipe &pipe);

	// timeout_ms: -1 = infinite
	void waitAndDispatch(int timeout_ms, void (*dispatcher)(Owner owner, int revents));

private:

	// Non-copyable
	Poller(const Poller&);
	// Non-assignable
	Poller& operator=(const Poller&);

	void add(int fd, int events, Owner owner);
	void mod(int fd, int events);
	void remove(int fd);

	std::vector<struct pollfd> pfds_;        // passed directly to poll()
	std::vector<Owner>         owners_;      // parallel to pfds_
	std::vector<int>           fd_to_index_; // fd → index in pfds_

	std::vector<std::pair<Owner,int> > ready_; // temporary storage for ready events

};
