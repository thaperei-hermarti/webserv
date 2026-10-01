#include "logger/Logger.hpp"

#include <iostream>
#include <unistd.h>

static const char* getLevelName(Logger::Level level)
{
	switch (level)
	{
		case Logger::DEBUG:
			return "DEBUG";
		case Logger::INFO:
			return "INFO";
		case Logger::WARNING:
			return "WARNING";
		case Logger::ERROR:
			return "ERROR";
	}
	return "UNKNOWN";
}

static const char* getLevelColor(Logger::Level level)
{
	switch (level)
	{
		case Logger::DEBUG:
			return "\033[36m";
		case Logger::INFO:
			return "\033[32m";
		case Logger::WARNING:
			return "\033[33m";
		case Logger::ERROR:
			return "\033[31m";
	}
	return "\033[0m";
}

void Logger::log(Level level, const std::string& message)
{
	const bool use_color = isatty(STDERR_FILENO) != 0;
	if (use_color)
		std::cerr << getLevelColor(level);
	std::cerr << "[" << getLevelName(level) << "]";
	if (use_color)
		std::cerr << "\033[0m";
	std::cerr << " " << message << std::endl;
}

void Logger::debug(const std::string& message)
{
	log(DEBUG, message);
}

void Logger::info(const std::string& message)
{
	log(INFO, message);
}

void Logger::warning(const std::string& message)
{
	log(WARNING, message);
}

void Logger::error(const std::string& message)
{
	log(ERROR, message);
}
