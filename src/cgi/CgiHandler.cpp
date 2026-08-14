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

void CgiHandler::buildEnv(HttpRequest& request, LocationConfig& config)
{
	(void) request;
	(void) config;
}

void CgiHandler::unchunkAndFeedStdin()
{
}
