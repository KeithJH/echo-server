#pragma once

#include <EchoServer/Logger.hpp>

namespace EchoServer
{
class SocketClient
{
	public:
	SocketClient(Logger *logger, int fd);
	~SocketClient();

	bool ReadAndWrite();

	private:
	Logger *_logger;
	int _clientFd;
	char _buffer[1 << 20]; // TODO: This has potential for re-use
};
} // namespace EchoServer
