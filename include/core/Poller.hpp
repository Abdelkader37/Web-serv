#pragma once

#include "Socket.hpp"      // Socket
#include "Pipe.hpp"        // Pipe
#include <vector>          // std::vector
#include <poll.h>         // struct pollfd

class Poller
{

public:

	struct Handle
	{
		void *owner;
		int   revents;
	};

	Poller();
	~Poller();

	void addHandle(Socket &socket, int events);
	void addHandle(Pipe &pipe, int events);

	void modHandle(Socket &socket, int events);
	void modHandle(Pipe &pipe, int events);

	void removeHandle(Socket &socket);
	void removeHandle(Pipe &pipe);

	// Returns ready Handles, empty if timeout expires.
	std::vector<Handle> wait(int timeout_ms);

private:

	// Non-copyable (one instance of Poller per lifetime of the program)
	Poller(const Poller&);
	// Non-assignable (one instance of Poller per lifetime of the program)
	Poller& operator=(const Poller&);

	std::vector<struct pollfd> pfds_;   // passed directly to poll()
	std::vector<void *>        owners_; // same index = same handle

};
