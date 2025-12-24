#include <EchoServer/Logger.hpp>
#include <EchoServer/SocketServer.hpp>
#include <netinet/in.h>
#include <sys/un.h>

namespace EchoServer
{
// ----------------------------------------------------------------------------
// SocketServer ---------------------------------------------------------------
// ----------------------------------------------------------------------------
SocketServer::SocketServer(Logger *logger) : _logger(logger) {}

SocketServer::~SocketServer()
{
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

std::unique_ptr<SocketClient> SocketServer::AcceptClient()
{
	int clientFd = ::accept(_socketFd, nullptr, nullptr);
	if (clientFd == -1)
	{
		_logger->PrintError("Failed to accept a client connection\n");
		return nullptr;
	}

	return std::make_unique<SocketClient>(_logger, clientFd);
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

	return true;
}

// ----------------------------------------------------------------------------
// InetSocketServer -----------------------------------------------------------
// ----------------------------------------------------------------------------
InetSocketServer::InetSocketServer(Logger *logger) : SocketServer(logger) {}
bool InetSocketServer::Initialize(unsigned int address, unsigned short port)
{
	_socketFd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (_socketFd == -1)
	{
		_logger->PrintError("Could not create server socket\n");
		return false;
	}

	sockaddr_in serverAddress{};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = address;

	return InternalInitialize(reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));
}

// ----------------------------------------------------------------------------
// UnixSocketServer -----------------------------------------------------------
// ----------------------------------------------------------------------------
UnixSocketServer::UnixSocketServer(Logger *logger, std::filesystem::path path) : SocketServer(logger), _socketPath(path)
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
