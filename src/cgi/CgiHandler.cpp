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

#include "cgi/CgiHandler.hpp"
#include <cctype>
#include <sstream>

CgiHandler::CgiHandler() : pid_(-1)
{
	stdin_pipe_[0] = -1;
	stdin_pipe_[1] = -1;
	stdout_pipe_[0] = -1;
	stdout_pipe_[1] = -1;
}

CgiHandler::~CgiHandler()
{
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
	}
	return *this;
}

int CgiHandler::getFd() const
{
	return stdout_pipe_[0];
}

void CgiHandler::handleReadEvent()
{
}

void CgiHandler::handleWriteEvent()
{
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
	(void) request;
	(void) config;
	return HttpResponse();
}

void CgiHandler::execute(HttpRequest& request, LocationConfig& config)
{
	(void) request;
	(void) config;
}

static std::string toUpperWithUnderscores(const std::string& str)
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
