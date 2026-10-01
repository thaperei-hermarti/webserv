/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:20:30 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:20:31 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "logger/Logger.hpp"
#include "cgi/CgiHandler.hpp"
#include "server/Reactor.hpp"
#include <cctype>
#include <fcntl.h>
#include <signal.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

CgiHandler::CgiHandler() : pid_(-1)
{
	stdin_pipe_[0] = stdin_pipe_[1] = -1;
	stdout_pipe_[0] = stdout_pipe_[1] = -1;
	if (pipe(stdin_pipe_) < 0)
	{
		Logger::error("Stdin pipe");
		return;
	}
	if (pipe(stdout_pipe_) < 0)
	{
		Logger::error("Stdout pipe");
		closeFd(stdin_pipe_[0]);
		closeFd(stdin_pipe_[1]);
	}
}

//CgiHandler::CgiHandler(Reactor* reactor) : pid_(-1), reactor_(reactor)
//{
//	stdin_pipe_[0] = stdin_pipe_[1] = -1;
//	stdout_pipe_[0] = stdout_pipe_[1] = -1;
//	if (pipe(stdin_pipe_) < 0)
//	{
//		Logger::error("Stdin pipe");
//		return;
//	}
//	if (pipe(stdout_pipe_) < 0)
//	{
//		Logger::error("Stdout pipe");
//		closeFd(stdin_pipe_[0]);
//		closeFd(stdin_pipe_[1]);
//	}
//}

CgiHandler::~CgiHandler()
{
	if (pid_ > 0)
	{
		kill(pid_, SIGTERM);
		waitpid(pid_, NULL, 0);
	}
	closeFd(stdin_pipe_[0]);
	closeFd(stdin_pipe_[1]);
	closeFd(stdout_pipe_[0]);
	closeFd(stdout_pipe_[1]);
}

CgiHandler::CgiHandler(const CgiHandler& other)
	: pid_(other.pid_), env_(other.env_), input_buffer_(other.input_buffer_),
	  output_buffer_(other.output_buffer_)
{
	stdin_pipe_[0] = other.stdin_pipe_[0];
	stdin_pipe_[1] = other.stdin_pipe_[1];
	stdout_pipe_[0] = other.stdout_pipe_[0];
	stdout_pipe_[1] = other.stdout_pipe_[1];
}

CgiHandler& CgiHandler::operator=(const CgiHandler& other)
{
	if (this != &other)
	{
		pid_ = other.pid_;
		stdin_pipe_[0] = other.stdin_pipe_[0];
		stdin_pipe_[1] = other.stdin_pipe_[1];
		stdout_pipe_[0] = other.stdout_pipe_[0];
		stdout_pipe_[1] = other.stdout_pipe_[1];
		env_ = other.env_;
		input_buffer_ = other.input_buffer_;
		output_buffer_ = other.output_buffer_;
		//reactor_ = other.reactor_;
	}
	return *this;
}

int CgiHandler::getFd() const
{
	return stdout_pipe_[0];
}

void CgiHandler::handleReadEvent()
{
	char buffer[4096];
	ssize_t bytes_read = read(stdout_pipe_[0], buffer, sizeof(buffer));
	if (bytes_read > 0)
	{
		output_buffer_.append(buffer, bytes_read);
	}
	else if (bytes_read == 0)
	{
		//if (reactor_ != NULL)
		//	reactor_->unregisterHandler(stdout_pipe_[0]);
		closeFd(stdout_pipe_[0]);
		waitpid(pid_, NULL, 0);
		pid_ = -1;
	}
}

void CgiHandler::handleWriteEvent()
{
	if (input_buffer_.empty())
	{
		//if (reactor_ != NULL)
		//	reactor_->unregisterHandler(stdin_pipe_[1]);
		closeFd(stdin_pipe_[1]);
		return;
	}
	ssize_t bytes_written =
		write(stdin_pipe_[1], input_buffer_.c_str(), input_buffer_.size());
	if (bytes_written > 0)
	{
		input_buffer_.erase(0, bytes_written);
	}
	if (input_buffer_.empty())
	{
		//if (reactor_ != NULL)
		//	reactor_->unregisterHandler(stdin_pipe_[1]);
		closeFd(stdin_pipe_[1]);
	}
}

void CgiHandler::handleTimeout()
{
}

bool CgiHandler::wantsWrite() const
{
	return false;
}

HttpResponse CgiHandler::handle(HttpRequest& request, LocationConfig& config)
{
	std::string server_name = request.getHeader("host");
	std::size_t colon = server_name.find(':');
	if (colon != std::string::npos)
	{
		server_name.erase(colon);
	}
	buildEnv(request, config, server_name, 80);
	input_buffer_ = request.getBody();
	execute(request, config);
	//	if (reactor_ != NULL)
	//	{
	//		reactor_->registerHandler(stdin_pipe_[1], this, WRITE);
	//		reactor_->registerHandler(stdout_pipe_[0], this, READ);
	//	}
	return HttpResponse();
}

void CgiHandler::execute(HttpRequest& request, LocationConfig& config)
{
	std::string path = request.getUri();
	std::size_t query = path.find('?');
	path = path.substr(0, query);
	std::size_t extension = path.rfind('.');
	if (extension == std::string::npos)
	{
		Logger::warning("No extension");
		return;
	}
	std::map<std::string, std::string>::const_iterator interpreter =
		config.cgi_extensions_.find(path.substr(extension));
	if (interpreter == config.cgi_extensions_.end())
	{
		Logger::warning("No interpreter for cgi");
		return;
	}
	std::string script = config.root_ + path;
	pid_ = fork();
	if (pid_ < 0)
	{
		Logger::error("Forking process");
		return;
	}
	if (pid_ == 0)
	{
		dup2(stdin_pipe_[0], STDIN_FILENO);
		dup2(stdout_pipe_[1], STDOUT_FILENO);
		close(stdin_pipe_[0]);
		close(stdin_pipe_[1]);
		close(stdout_pipe_[0]);
		close(stdout_pipe_[1]);

		std::vector<char*> envp = toCharArray();
		char* argv[3];
		argv[0] = const_cast<char*>(interpreter->second.c_str());
		argv[1] = const_cast<char*>(script.c_str());
		argv[2] = NULL;
		execve(argv[0], argv, &envp[0]);
		_exit(127);
	}
	closeFd(stdin_pipe_[0]);
	closeFd(stdout_pipe_[1]);
	int flags = fcntl(stdin_pipe_[1], F_GETFL, 0);
	fcntl(stdin_pipe_[1], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(stdout_pipe_[0], F_GETFL, 0);
	fcntl(stdout_pipe_[0], F_SETFL, flags | O_NONBLOCK);
}

std::string CgiHandler::toUpperWithUnderscores(const std::string& str)
{
	std::string normalized_str(str);
	for (std::size_t i = 0; i < normalized_str.size(); ++i)
	{
		normalized_str[i] = static_cast<char>(
			toupper(static_cast<unsigned char>(normalized_str[i])));
	}
	for (std::size_t i = normalized_str.find('-'); i != std::string::npos;
		 i = normalized_str.find('-'))
	{
		normalized_str[i] = '_';
	}
	return normalized_str;
}

void CgiHandler::buildEnv(HttpRequest& request,
						  LocationConfig& config,
						  const std::string& server_name,
						  int server_port)
{
	env_.clear();
	env_.push_back("REQUEST_METHOD=" + request.getMethod());
	env_.push_back("SCRIPT_NAME=" + request.getUri());
	env_.push_back("SCRIPT_FILENAME=" + config.root_);
	env_.push_back("QUERY_STRING=" + request.getUri());
	env_.push_back("PATH_INFO=" + request.getUri());
	env_.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env_.push_back("SERVER_SOFTWARE=webserv/1.0");
	env_.push_back("GATEWAY_INTERFACE=CGI/1.1");

	env_.push_back("SERVER_NAME=" + server_name);
	std::ostringstream port_stream;
	port_stream << server_port;
	env_.push_back("SERVER_PORT=" + port_stream.str());

	if (request.getMethod() == "POST")
	{
		std::map<std::string, std::string>::const_iterator it =
			request.getHeaders().find("content-type");
		if (it != request.getHeaders().end())
		{
			env_.push_back("CONTENT_TYPE=" + it->second);
		}

		std::ostringstream length_stream;
		length_stream << request.getBody().size();
		env_.push_back("CONTENT_LENGTH=" + length_stream.str());
	}
	for (std::map<std::string, std::string>::const_iterator it =
			 request.getHeaders().begin();
		 it != request.getHeaders().end();
		 ++it)
	{
		std::string key = "HTTP_" + toUpperWithUnderscores(it->first);
		env_.push_back(key + "=" + it->second);
	}
}

std::vector<char*> CgiHandler::toCharArray()
{
	std::vector<char*> result;
	result.reserve(env_.size());

	for (std::size_t i = 0; i < env_.size(); ++i)
	{
		result.push_back(const_cast<char*>(env_[i].c_str()));
	}
	result.push_back(NULL);
	return result;
}

void CgiHandler::unchunkAndFeedStdin()
{
}

void CgiHandler::closeFd(int& fd)
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
}
