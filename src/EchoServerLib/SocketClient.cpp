#include <EchoServer/SocketClient.hpp>
#include <sys/socket.h>
#include <unistd.h>

namespace EchoServer
{
SocketClient::SocketClient(Logger *logger, int fd) : _logger(logger), _clientFd(fd) {}
SocketClient::~SocketClient()
{
	if (_clientFd != -1)
	{
		if (::close(_clientFd) == -1)
		{
			_logger->PrintError("Could not close client\n");
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
				_logger->PrintError("Failed to send bytes to client\n");
				return false;
			}

			writtenBytes += partialWrittenBytes;
		}
	}

	if (readBytes == 0)
	{
		_logger->PrintInfo("EOF from client connection\n");
		return true;
	}
	else
	{
		_logger->PrintError("Failed to read from client connection\n");
		return false;
	}
}
} // namespace EchoServer
