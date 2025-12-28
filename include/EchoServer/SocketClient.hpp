#pragma once

#include <EchoServer/Logger.hpp>
#include <memory>

namespace EchoServer
{
class SocketClient
{
	public:
	SocketClient(std::shared_ptr<Logger> &logger, int fd);
	~SocketClient();

	enum IoStatus
	{
		WouldBlock = -2,
		Failed = -1,
		Eof = 0
	};
	IoStatus ReadAndWriteBlocking();
	IoStatus ReadNonBlockingAndWriteBlocking();

	private:
	// TODO: It's not really necessary to keep this per client. Move functions to Server?
	std::shared_ptr<Logger> _logger;

	int _clientFd;
};
} // namespace EchoServer
