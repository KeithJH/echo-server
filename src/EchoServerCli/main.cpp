#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketClient.hpp>
#include <EchoServer/SocketServer.hpp>
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

static ConsoleLogger logger{};
static std::stop_source stopSource{};

static void signalHandler([[maybe_unused]] int signal)
{
	stopSource.request_stop();
	logger.PrintInfo("Program signaled to stop. Will continue until current iteration completes.\n");
}

static std::unique_ptr<EchoServer::SocketServer> CreateServerFromArgs(char **argv)
{
	switch (argv[1][0])
	{
	case 'i':
	case 'I':
		return std::make_unique<EchoServer::InetSocketServer>(&logger, INADDR_ANY, atoi(&argv[2][0]),
		                                                      stopSource.get_token());

	case 'u':
	case 'U':
		return std::make_unique<EchoServer::UnixSocketServer>(&logger, std::filesystem::path{&argv[2][0]},
		                                                      stopSource.get_token());
		break;

	default:
		logger.PrintError("Unknown socket type\n");
		return nullptr;
	}
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

	std::unique_ptr<EchoServer::SocketServer> server = CreateServerFromArgs(argv);

	if (!server || !server->Initialize())
		return 1;

	signal(SIGINT, signalHandler);
	while (!stopSource.stop_requested())
	{
		if (!server->LoopIteration())
			return 1;
	}

	return 0;
}
