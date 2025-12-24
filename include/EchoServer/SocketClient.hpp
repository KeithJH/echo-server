#pragma once

namespace EchoServer {
class SocketClient
{
	public:
	SocketClient(int fd);
	~SocketClient();

	bool ReadAndWrite();

	private:
	int _clientFd;
	char _buffer[1 << 20]; // TODO: This has potential for re-use
};
}
