#pragma once

#include <filesystem>
#include <memory>
#include <sys/socket.h>

#include "SocketClient.hpp"

class SocketServer
{
	public:
	virtual ~SocketServer();
	std::unique_ptr<SocketClient> AcceptClient();

	protected:
	bool InternalInitialize(const sockaddr *socketAddress, const socklen_t socketAddressSize);

	protected:
	int _socketFd = -1;
};

class InetSocketServer : public SocketServer
{
	public:
	bool Initialize(unsigned int address, unsigned short port);
};

class UnixSocketServer : public SocketServer
{
	public:
	UnixSocketServer(std::filesystem::path path);
	virtual ~UnixSocketServer();

	bool Initialize();

	private:
	std::filesystem::path _socketPath;
};
