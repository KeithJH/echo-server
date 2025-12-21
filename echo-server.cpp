#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

void printInfo(const char *info) { std::fputs(info, stdout); }
void printError(const char *error) { std::fputs(error, stderr); }

int main()
{
	int socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1)
	{
		printError("Could not create server socket\n");
		return 1;
	}

	sockaddr_in serverAddress {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(9090);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (::bind(socketFd, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)
	{
		printError("Could not bind to server socket\n");
		::close(socketFd);
		return 1;
	}

	if (::listen(socketFd, 0) == -1)
	{
		printError("Could not listen on server socket\n");
		::close(socketFd);
		return 1;
	}

	int clientFd = ::accept(socketFd, nullptr, nullptr);
	if (clientFd == -1)
	{
		printError("Failed to accept a client connection\n");
		::close(socketFd);
		return 1;
	}

	char connectionBuffer[1024];
	ssize_t readBytes = ::recv(clientFd, connectionBuffer, sizeof(connectionBuffer), 0);
	for (; readBytes > 0; readBytes = ::recv(clientFd, connectionBuffer, sizeof(connectionBuffer), 0))
	{
		ssize_t writtenBytes {0};
		while (writtenBytes < readBytes)
		{
			ssize_t partialWrittenBytes = ::send(clientFd, connectionBuffer + writtenBytes, static_cast<size_t>(readBytes - writtenBytes), 0);
			if (partialWrittenBytes <= 0)
			{
				printError("Failed to send bytes to client\n");
				::close(clientFd);
				::close(socketFd);
				return 1;
			}

			writtenBytes += partialWrittenBytes;
		}
	}

	if (readBytes == 0)
	{
		printInfo("EOF from client connection\n");
	}
	else if (readBytes == -1)
	{
		printError("Failed to read from client connection\n");
		::close(clientFd);
		::close(socketFd);
		return 1;
	}

	if (::close(clientFd) == -1)
	{
		printError("Failed to close client connection\n");
	}

	if (::close(socketFd) == -1)
	{
		printError("Failed to close server socket\n");
	}

	return 0;
}
