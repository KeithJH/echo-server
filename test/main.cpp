#include <EchoServer/SocketServer.hpp>
#include <iostream>
#include <netinet/in.h>

class ConsoleLogger : public EchoServer::Logger
{
	public:
	void PrintInfo(const char *info) override { std::cout << info; }
	void PrintError(const char *error) override { std::cerr << error; }
};
static std::shared_ptr<ConsoleLogger> logger = std::make_shared<ConsoleLogger>();

bool InitializeUnixSocketServer()
{
	std::stop_source stopSource{};
	EchoServer::UnixSocketServer server{logger, stopSource.get_token(), std::filesystem::path{"sock"}};

	stopSource.request_stop();
	return server.Initialize();
}

bool InitializeInetSocketServer()
{
	std::stop_source stopSource{};
	EchoServer::InetSocketServer server{logger, stopSource.get_token(), INADDR_ANY, 9090};

	stopSource.request_stop();
	return server.Initialize();
}

int main()
{
	if (!InitializeUnixSocketServer())
		return 1;

	if (!InitializeInetSocketServer())
		return 1;

	return 0;
}
