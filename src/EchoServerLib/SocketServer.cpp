#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketServer.hpp>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

namespace EchoServer
{
// ----------------------------------------------------------------------------
// SocketServer ---------------------------------------------------------------
// ----------------------------------------------------------------------------
SocketServer::SocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken) : _logger(logger), _stopToken(stopToken) {}

SocketServer::~SocketServer()
{
	// Allow all the threads to stop
	// TODO: writes are still blocking, so if a client doesn't read we'll still wait...
	{
		std::unique_lock lock(_clientMutex);

		_clientsToRequeue.clear();
		_clientsWithPendingReads.clear();
		_clientCondition.notify_all();
	}

	for (size_t i = 0; i < MAX_WORKER_THREADS; i++)
	{
		_clientWorkers[i].join();
	}

	if (_socketFd != -1)
	{
		if (::close(_socketFd) == -1)
		{
			_logger->PrintError("Could not close server socket\n");
		}
		else
		{
			_logger->PrintInfo("Socket closed\n");
		}
	}
}

bool SocketServer::AcceptClient()
{
	int clientFd = ::accept(_socketFd, nullptr, nullptr);
	if (clientFd == -1)
	{
		_logger->PrintError("Failed to accept a client connection\n");
		return false;
	}

	epoll_event epollEvent{};
	epollEvent.events = EPOLLIN | EPOLLET;
	epollEvent.data.fd = clientFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &epollEvent) == -1)
	{
		_logger->PrintError("Could not add client to polling mechanism\n");
		::close(clientFd);
		return false;
	}

	_clients.try_emplace(clientFd, _logger, clientFd);
	return true;
}

void SocketServer::ThreadFunc()
{
	while (!_stopToken.stop_requested())
	{
		int clientFd = -1;
		{
			std::unique_lock lock(_clientMutex);

			if (_clientsWithPendingReads.empty())
			{
				_clientCondition.wait(lock,
				                      [&] { return !_clientsWithPendingReads.empty() || _stopToken.stop_requested(); });

				if (_stopToken.stop_requested())
					return;
			}

			clientFd = _clientsWithPendingReads.extract(std::begin(_clientsWithPendingReads)).value();
			_clientsInProgress.emplace(clientFd);
		}

		auto mappedClient = _clients.find(clientFd);
		if (mappedClient != _clients.end())
		{
			switch (mappedClient->second.ReadNonBlockingAndWriteBlocking())
			{
			case SocketClient::WouldBlock:
			case SocketClient::Failed:
				break;
			case SocketClient::Eof:
				_clients.erase(clientFd);
				break;
			}
		}

		{
			std::unique_lock lock(_clientMutex);
			auto inProgressNode = _clientsInProgress.extract(clientFd);
			auto requeueNode = _clientsToRequeue.extract(clientFd);
			if (!requeueNode.empty())
			{
				// TODO: just handle it ourself, skipping taking the lock again?
				_clientsWithPendingReads.emplace(clientFd);
			}
		}
	}
}

bool SocketServer::LoopIteration()
{
	int numPolledEvents = epoll_wait(_epollFd, _epollEvents, MAX_EPOLL_EVENTS, -1);
	if (numPolledEvents == -1)
	{
		_logger->PrintError("Failed while waiting on polling mechanism\n");
		return true; // Not critical? Retry.
	}

	for (int i = 0; i < numPolledEvents; i++)
	{
		if (_epollEvents[i].data.fd == _socketFd)
		{
			[[maybe_unused]] bool result = AcceptClient();
		}
		else
		{
			const int clientFd = _epollEvents[i].data.fd;
			std::unique_lock lock(_clientMutex);

			if (_clientsInProgress.contains(clientFd))
			{
				_clientsToRequeue.emplace(clientFd);
			}
			else
			{
				_clientsWithPendingReads.emplace(clientFd);
				_clientCondition.notify_one();
			}
		}
	}

	return true;
}

bool SocketServer::InternalInitialize(const sockaddr *socketAddress, const socklen_t socketAddressSize)
{
	if (::bind(_socketFd, socketAddress, socketAddressSize) == -1)
	{
		_logger->PrintError("Could not bind to server socket\n");
		return false;
	}

	if (::listen(_socketFd, 0) == -1)
	{
		_logger->PrintError("Could not listen on server socket\n");
		return false;
	}

	_epollFd = epoll_create1(0);
	if (_epollFd == -1)
	{
		_logger->PrintError("Could not create polling mechanism\n");
		return false;
	}

	epoll_event epollEvent{};
	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = _socketFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, _socketFd, &epollEvent) == -1)
	{
		_logger->PrintError("Could not add socket to polling mechanism\n");
		return false;
	}

	for (size_t i = 0; i < MAX_WORKER_THREADS; i++)
	{
		_clientWorkers[i] = std::thread(&SocketServer::ThreadFunc, this);
	}

	return true;
}

// ----------------------------------------------------------------------------
// InetSocketServer -----------------------------------------------------------
// ----------------------------------------------------------------------------
InetSocketServer::InetSocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken, unsigned int address, unsigned short port)
	: SocketServer(logger, stopToken), _address(address), _port(port)
{
}
bool InetSocketServer::Initialize()
{
	_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (_socketFd == -1)
	{
		_logger->PrintError("Could not create server socket\n");
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(_port);
	serverAddress.sin_addr.s_addr = _address;

	return InternalInitialize(reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));
}

// ----------------------------------------------------------------------------
// UnixSocketServer -----------------------------------------------------------
// ----------------------------------------------------------------------------
UnixSocketServer::UnixSocketServer(std::shared_ptr<Logger> logger, std::stop_token stopToken, std::filesystem::path path)
	: SocketServer(logger, stopToken), _socketPath(path)
{
}

UnixSocketServer::~UnixSocketServer()
{
	if (!_socketPath.empty())
	{
		if (::unlink(_socketPath.c_str()) == -1)
		{
			_logger->PrintError("Could not unlink socket path\n");
		}
		else
		{
			_logger->PrintInfo("Unix socket unlinked\n");
		}
	}
}

bool UnixSocketServer::Initialize()
{
	_socketFd = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (_socketFd == -1)
	{
		_logger->PrintError("Could not create server socket\n");
		return false;
	}

	sockaddr_un serverAddress{};
	serverAddress.sun_family = AF_UNIX;

	std::string_view pathStringView{_socketPath.native()};
	strncpy(serverAddress.sun_path, pathStringView.data(), pathStringView.length());

	return InternalInitialize(reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));
}
} // namespace EchoServer
