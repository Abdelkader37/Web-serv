#pragma once

class Socket
{

public:
	Socket();
	Socket(int fd);

	int get() const;

	~Socket();
// Non-copyable
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);

	int fd_;

};
