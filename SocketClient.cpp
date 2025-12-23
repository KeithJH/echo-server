#include <sys/socket.h>
#include <unistd.h>

#include "SocketClient.hpp"
#include "util.hpp"

SocketClient::SocketClient(int fd) : _clientFd(fd) {}
SocketClient::~SocketClient()
{
	if (_clientFd != -1)
	{
		if (::close(_clientFd) == -1)
		{
			printError("Could not close client\n");
		}
	}
}

bool SocketClient::ReadAndWrite()
{
	ssize_t readBytes = ::recv(_clientFd, _buffer, sizeof(_buffer), 0);
	for (; readBytes > 0; readBytes = ::recv(_clientFd, _buffer, sizeof(_buffer), 0))
	{
		ssize_t writtenBytes{0};
		while (writtenBytes < readBytes)
		{
			ssize_t partialWrittenBytes =
				::send(_clientFd, _buffer + writtenBytes, static_cast<size_t>(readBytes - writtenBytes), 0);
			if (partialWrittenBytes <= 0)
			{
				printError("Failed to send bytes to client\n");
				return false;
			}

			writtenBytes += partialWrittenBytes;
		}
	}

	if (readBytes == 0)
	{
		printInfo("EOF from client connection\n");
		return true;
	}
	else
	{
		printError("Failed to read from client connection\n");
		return false;
	}
}
