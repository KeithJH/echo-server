#include <EchoServer/SocketClient.hpp>
#include <EchoServer/SocketServer.hpp>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// TODO: Handle this better. Could register this with an EchoServer::Logger?
void printInfo(const char *info) { std::fputs(info, stdout); }
void printError(const char *error) { std::fputs(error, stderr); }

static std::atomic<bool> handlingClients = true;
void signalHandler([[maybe_unused]] int signal)
{
	handlingClients = false;
	printInfo("Program signaled to stop. Will continue until current iteration completes\n");
}

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

	// TODO: Better stop condition
	// TODO: Handle multiple clients at once
	signal(SIGINT, signalHandler);
	while (handlingClients)
	{
		std::unique_ptr<EchoServer::SocketClient> client = server.AcceptClient();
		if (!client)
			return 1;

		if (!client->ReadAndWrite())
			return 1;
	}

	return 0;
}
