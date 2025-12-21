#include <cstdio>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void printInfo(const char *info) { std::fputs(info, stdout); }
void printError(const char *error) { std::fputs(error, stderr); }

class SocketClient
{
	public:
	SocketClient(int fd) : _clientFd(fd) {}
	~SocketClient()
	{
		if (_clientFd != -1)
		{
			if (::close(_clientFd) == -1)
			{
				printError("Could not close client\n");
			}
		}
	}

	bool ReadAndWrite()
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

	private:
	int _clientFd;
	char _buffer[1 << 20];	//TODO: This has potential for re-use
};

class SocketServer
{
	public:
	SocketServer() {}
	~SocketServer()
	{
		if (_socketFd != -1)
		{
			if (::close(_socketFd) == -1)
			{
				printError("Could not close server socket\n");
			}
		}
	}

	// TODO: Should be easy to add Unix Socket initialization as well
	bool Initialize(unsigned int address, unsigned short port)
	{
		_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
		if (_socketFd == -1)
		{
			printError("Could not create server socket\n");
			return false;
		}

		sockaddr_in serverAddress{};
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		serverAddress.sin_addr.s_addr = address;

		if (::bind(_socketFd, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
		{
			printError("Could not bind to server socket\n");
			return false;
		}

		if (::listen(_socketFd, 0) == -1)
		{
			printError("Could not listen on server socket\n");
			return false;
		}

		return true;
	}

	std::unique_ptr<SocketClient> AcceptClient()
	{
		int clientFd = ::accept(_socketFd, nullptr, nullptr);
		if (clientFd == -1)
		{
			printError("Failed to accept a client connection\n");
			return nullptr;
		}

		return std::make_unique<SocketClient>(clientFd);
	}

	private:
	int _socketFd = -1;
};

int main()
{
	SocketServer server{};
	if (!server.Initialize(INADDR_ANY, 9090))
		return 1;

	// TODO: Loop over multiple clients, with a signal to quit
	{
		std::unique_ptr<SocketClient> client = server.AcceptClient();
		if (!client)
			return 1;

		if (!client->ReadAndWrite())
			return 1;
	}

	return 0;
}
