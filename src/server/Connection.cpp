/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                    :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/Connection.hpp"

#include "cgi/CgiHandler.hpp"
#include "config/ServerConfig.hpp"

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

int Connection::getFd() const
{
	return fd_;
}

void Connection::handleReadEvent()
{
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
