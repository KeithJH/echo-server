#include <EchoServer/SocketClient.hpp>
#include <EchoServer/SocketServer.hpp>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main()
{
#if 0
	EchoServer::UnixSocketServer server{std::filesystem::path{"/dev/shm/my.sock"}};
	if (!server.Initialize())
		return 1;
#else
	EchoServer::InetSocketServer server{};
	if (!server.Initialize(INADDR_ANY, 9090))
		return 1;
#endif

	// TODO: Loop over multiple clients, with a signal to quit
	{
		std::unique_ptr<EchoServer::SocketClient> client = server.AcceptClient();
		if (!client)
			return 1;

		if (!client->ReadAndWrite())
			return 1;
	}

	return 0;
}
