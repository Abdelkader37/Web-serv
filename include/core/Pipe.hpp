#pragma once

class Pipe
{

public:

	Pipe();
	~Pipe();

	int getReadFd() const;
	int getWriteFd() const;

	void closeWriteFd();
	void closeReadFd();

private:

	// Non-copyable
	Pipe(const Pipe&);
	// Non-movable
	Pipe& operator=(const Pipe&);

	int readFd_;
	int writeFd_;

};
