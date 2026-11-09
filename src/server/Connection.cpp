/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:09 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:10 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Connection.hpp"

#include "cgi/CgiHandler.hpp"
#include "config/ServerConfig.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>

Connection::Connection(int fd, ServerConfig& config)
	: fd_(fd), state_(READING_HEADERS),
	  request_parser_(config.getClientMaxBodySize()), current_request_(NULL),
	  current_response_(NULL), server_config_(config), cgi_handler_(NULL),
	  last_activity_(time(NULL)), keep_alive_(false)
{
}

Connection::~Connection()
{
	delete cgi_handler_;
}

Connection::Connection(const Connection& other)
	: fd_(other.fd_), state_(other.state_), in_buffer_(other.in_buffer_),
	  out_buffer_(other.out_buffer_), request_parser_(other.request_parser_),
	  current_request_(other.current_request_),
	  current_response_(other.current_response_),
	  server_config_(other.server_config_), cgi_handler_(other.cgi_handler_),
	  last_activity_(other.last_activity_), keep_alive_(other.keep_alive_)
{
}

Connection& Connection::operator=(const Connection& other)
{
	if (this != &other)
	{
		fd_ = other.fd_;
		state_ = other.state_;
		in_buffer_ = other.in_buffer_;
		out_buffer_ = other.out_buffer_;
		request_parser_ = other.request_parser_;
		current_request_ = other.current_request_;
		current_response_ = other.current_response_;
		cgi_handler_ = other.cgi_handler_;
		last_activity_ = other.last_activity_;
		keep_alive_ = other.keep_alive_;
	}
	return *this;
}

const std::string& Connection::getInBuffer() const
{
	return in_buffer_;
}

int Connection::getFd() const
{
	return fd_;
}

void Connection::handleReadEvent()
{
	static const std::size_t READ_CHUNK = 8192;
	char buffer[READ_CHUNK];
	ssize_t bytes_read = 0;

	while (true)
	{
		bytes_read = recv(fd_, buffer, READ_CHUNK, 0);
		if (bytes_read == 0)
		{
			break;
		}
		if (bytes_read < 0)
		{
			break;
		}
		std::cout << bytes_read << '\n' << std::flush;
		in_buffer_.append(buffer, bytes_read);
	}
}

void Connection::handleWriteEvent()
{
}

void Connection::handleTimeout()
{
}

bool Connection::wantsWrite() const
{
	return keep_alive_;
}

void Connection::processRequest()
{
}

void Connection::sendResponse(HttpResponse& response)
{
	(void) response;
}
