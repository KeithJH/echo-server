#pragma once

#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketClient.hpp>
#include <filesystem>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unordered_map>

namespace EchoServer
{
class SocketServer
{
	public:
	SocketServer(Logger *logger);
	virtual ~SocketServer();
	virtual bool Initialize() = 0;
	bool AcceptClient();
	bool LoopIteration();

	protected:
	bool InternalInitialize(const sockaddr *socketAddress, const socklen_t socketAddressSize);

	public:
	static constexpr size_t MAX_EPOLL_EVENTS = 10;

	protected:
	Logger *_logger;
	int _socketFd = -1;
	int _epollFd = -1;
	epoll_event _epollEvents[MAX_EPOLL_EVENTS];
	std::unordered_map<int, SocketClient> _clients;
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
