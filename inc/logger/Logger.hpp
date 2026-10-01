#ifndef WEBSERV_LOGGER_LOGGER_HPP
#define WEBSERV_LOGGER_LOGGER_HPP

#include <string>

class Logger
{
  public:
	enum Level
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR
	};

	static void log(Level level, const std::string& message);
	static void debug(const std::string& message);
	static void info(const std::string& message);
	static void warning(const std::string& message);
	static void error(const std::string& message);

  private:
	Logger();
};

#endif
