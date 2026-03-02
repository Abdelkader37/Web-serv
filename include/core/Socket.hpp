#pragma once

class Socket
{

public:

	Socket();
	Socket(int fd);

	int get() const;

	~Socket();

private:

	// Non-copyable
	Socket(const Socket&);
	// Non-movable
	Socket& operator=(const Socket&);

	int fd_;

};
