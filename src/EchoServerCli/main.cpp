#include <EchoServer/Logger.hpp>
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

class ConsoleLogger : public EchoServer::Logger
{
	public:
	void PrintInfo(const char *info) override { std::fputs(info, stdout); }
	void PrintError(const char *error) override { std::fputs(error, stderr); }
};

ConsoleLogger logger{};
static std::atomic<bool> handlingClients = true;

void signalHandler([[maybe_unused]] int signal)
{
	handlingClients = false;
	logger.PrintInfo("Program signaled to stop. Will continue until current iteration completes.\n");
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		logger.PrintError("Usage: echo-server <socket_type> <argument>\n");
		logger.PrintError("echo-server inet <port>\n");
		logger.PrintError("echo-server unix <path>\n");
		return 1;
	}

	std::unique_ptr<EchoServer::SocketServer> server;
	switch (argv[1][0])
	{
	case 'i':
	case 'I':
		server = std::make_unique<EchoServer::InetSocketServer>(&logger, INADDR_ANY, atoi(&argv[2][0]));
		break;

	case 'u':
	case 'U':
		server = std::make_unique<EchoServer::UnixSocketServer>(&logger, std::filesystem::path{&argv[2][0]});
		break;

	default:
		logger.PrintError("Unknown socket type\n");
		return 1;
	}

	if (!server->Initialize())
		return 1;

	// TODO: Better stop condition
	// TODO: Handle multiple clients at once
	signal(SIGINT, signalHandler);
	while (handlingClients)
	{
		std::unique_ptr<EchoServer::SocketClient> client = server->AcceptClient();
		if (!client)
			return 1;

		if (!client->ReadAndWrite())
			return 1;
	}

	return 0;
}
