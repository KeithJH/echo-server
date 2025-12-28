#pragma once

#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketClient.hpp>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <stop_token>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace EchoServer
{
class SocketServer
{
	public:
	SocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken);
	virtual ~SocketServer();
	virtual bool Initialize() = 0;
	bool AcceptClient();
	bool LoopIteration();

	protected:
	bool InternalInitialize(const sockaddr *socketAddress, const socklen_t socketAddressSize);
	void ThreadFunc();

	public:
	static constexpr size_t MAX_EPOLL_EVENTS = 10;
	static constexpr size_t MAX_WORKER_THREADS = 4;

	protected:
	std::shared_ptr<Logger> _logger;
	int _socketFd = -1;

	private:
	int _epollFd = -1;
	epoll_event _epollEvents[MAX_EPOLL_EVENTS];

	std::thread _clientWorkers[MAX_WORKER_THREADS];
	std::stop_token _stopToken;
	std::mutex _clientMutex;
	std::condition_variable _clientCondition;

	std::unordered_map<int, SocketClient> _clients;
	std::unordered_set<int> _clientsWithPendingReads;
	std::unordered_set<int> _clientsInProgress;
	std::unordered_set<int> _clientsToRequeue;
};

class InetSocketServer : public SocketServer
{
	public:
	InetSocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken, unsigned int address, unsigned short port);
	bool Initialize() override;

	unsigned int _address;
	unsigned short _port;
};

class UnixSocketServer : public SocketServer
{
	public:
	UnixSocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken, std::filesystem::path path);
	virtual ~UnixSocketServer();

	bool Initialize() override;

	private:
	std::filesystem::path _socketPath;
};
} // namespace EchoServer
