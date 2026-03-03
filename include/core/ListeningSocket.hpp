#pragma once

#include "core/Socket.hpp" // Socket class
#include <string>          // std::string

class ListeningSocket : public Socket
{

public:

	ListeningSocket(const std::string &iface, int port);

	int acceptConnection();

private:
	// Non-copyable
	ListeningSocket(const ListeningSocket &other);
	// Non-assignable
	ListeningSocket &operator=(const ListeningSocket &other);

};
