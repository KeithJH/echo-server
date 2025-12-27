#pragma once

#include <EchoServer/Logger.hpp>

namespace EchoServer
{
class SocketClient
{
	public:
	SocketClient(Logger *logger, int fd);
	~SocketClient();

	enum IoStatus { WouldBlock = -2, Failed = -1, Eof = 0 };
	IoStatus ReadAndWriteBlocking();
	IoStatus ReadNonBlockingAndWriteBlocking();

	private:
	Logger *_logger;
	int _clientFd;
	char _buffer[1 << 20]; // TODO: This has potential for re-use
};
} // namespace EchoServer
