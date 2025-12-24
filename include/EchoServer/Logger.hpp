#pragma once

namespace EchoServer
{
class Logger
{
	public:
	virtual ~Logger() = 0;
	virtual void PrintInfo(const char *info) = 0;
	virtual void PrintError(const char *error) = 0;
};
} // namespace EchoServer
