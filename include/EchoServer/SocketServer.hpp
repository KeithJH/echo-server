#pragma once

#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketClient.hpp>
#include <filesystem>
#include <memory>
#include <sys/socket.h>

namespace EchoServer
{
class SocketServer
{
	public:
	SocketServer(Logger *logger);
	virtual ~SocketServer();
	virtual bool Initialize() = 0;
	std::unique_ptr<SocketClient> AcceptClient();

	protected:
	bool InternalInitialize(const sockaddr *socketAddress, const socklen_t socketAddressSize);

	protected:
	Logger *_logger;
	int _socketFd = -1;
};

class InetSocketServer : public SocketServer
{
	public:
	InetSocketServer(Logger *logger, unsigned int address, unsigned short port);
	bool Initialize() override;

	unsigned int _address;
	unsigned short _port;
};

class UnixSocketServer : public SocketServer
{
	public:
	UnixSocketServer(Logger *logger, std::filesystem::path path);
	virtual ~UnixSocketServer();

	bool Initialize() override;

	private:
	std::filesystem::path _socketPath;
};
} // namespace EchoServer
