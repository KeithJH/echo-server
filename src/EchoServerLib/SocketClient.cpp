#include <EchoServer/SocketClient.hpp>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

namespace EchoServer
{
SocketClient::SocketClient(std::shared_ptr<Logger> &logger, int fd) : _logger(logger), _clientFd(fd) {}
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

SocketClient::IoStatus SocketClient::ReadAndWriteBlocking()
{
	// We always empty the buffer or error out,
	// so no need to retain across calls
	char buffer[1 << 20];

	ssize_t readBytes = ::recv(_clientFd, buffer, sizeof(buffer), 0);
	for (; readBytes > 0; readBytes = ::recv(_clientFd, buffer, sizeof(buffer), 0))
	{
		ssize_t writtenBytes{0};
		while (writtenBytes < readBytes)
		{
			ssize_t partialWrittenBytes =
				::send(_clientFd, buffer + writtenBytes, static_cast<size_t>(readBytes - writtenBytes), 0);
			if (partialWrittenBytes <= 0)
			{
				_logger->PrintError("Failed to send bytes to client\n");
				return SocketClient::IoStatus::Failed;
			}

			writtenBytes += partialWrittenBytes;
		}
	}

	if (readBytes == 0)
	{
		_logger->PrintInfo("EOF from client connection\n");
		return SocketClient::IoStatus::Eof;
	}
	else
	{
		_logger->PrintError("Failed to read from client connection\n");
		return SocketClient::IoStatus::Failed;
	}
}

SocketClient::IoStatus SocketClient::ReadNonBlockingAndWriteBlocking()
{
	// We always empty the buffer or error out,
	// so no need to retain across calls
	char buffer[1 << 20];

	ssize_t readBytes = ::recv(_clientFd, buffer, sizeof(buffer), MSG_DONTWAIT);
	for (; readBytes > 0; readBytes = ::recv(_clientFd, buffer, sizeof(buffer), MSG_DONTWAIT))
	{
		ssize_t writtenBytes{0};
		while (writtenBytes < readBytes)
		{
			ssize_t partialWrittenBytes =
				::send(_clientFd, buffer + writtenBytes, static_cast<size_t>(readBytes - writtenBytes), 0);
			if (partialWrittenBytes <= 0)
			{
				_logger->PrintError("Failed to send bytes to client\n");
				return SocketClient::IoStatus::Failed;
			}

			writtenBytes += partialWrittenBytes;
		}
	}

	switch (readBytes)
	{
	case -1:
#if EWOULDBLOCK != EAGAIN // Avoid warning for "logical 'or' of equal expressions, but remain portable
		if (errno == EWOULDBLOCK || errno == EAGAIN)
#else
		if (errno == EWOULDBLOCK)
#endif
		{
			return SocketClient::IoStatus::WouldBlock;
		}
		else
		{
			_logger->PrintError("Failed to read from client connection\n");
			return SocketClient::IoStatus::Failed;
		}
	case 0:
		_logger->PrintInfo("EOF from client connection\n");
		return SocketClient::IoStatus::Eof;
	default:
		_logger->PrintError("Unknown error with client IO\n");
		return SocketClient::IoStatus::Failed; // Shouldn't get here
	}
}

} // namespace EchoServer
