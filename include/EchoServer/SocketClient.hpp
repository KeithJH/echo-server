#pragma once

#include <EchoServer/Logger.hpp>

namespace EchoServer
{
class SocketClient
{
	public:
	SocketClient(Logger *logger, int fd);
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
	Logger *_logger;

	int _clientFd;
};
} // namespace EchoServer
