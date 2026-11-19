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

#include "http/response/HttpResponse.hpp"
#include "server/EventType.hpp"
#include "server/ConnState.hpp"
#include "server/Reactor.hpp"
#include "cgi/CgiHandler.hpp"
#include "config/ServerConfig.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

Connection::Connection(int fd, ServerConfig& config, Reactor& reactor)
	: fd_(fd), state_(READING_HEADERS),
	  request_parser_(config.getClientMaxBodySize()), current_request_(NULL),
	  current_response_(NULL), server_config_(config), reactor_(&reactor),
	  router_(), cgi_handler_(NULL), last_activity_(time(NULL)),
	  keep_alive_(false)
{
}

Connection::~Connection()
{
	delete current_response_;
	delete cgi_handler_;
}

Connection::Connection(const Connection& other)
	: fd_(other.fd_), state_(other.state_), in_buffer_(other.in_buffer_),
	  out_buffer_(other.out_buffer_), request_parser_(other.request_parser_),
	  current_request_(other.current_request_),
	  current_response_(other.current_response_
							? new HttpResponse(*other.current_response_)
							: NULL),
	  server_config_(other.server_config_), reactor_(other.reactor_),
	  router_(other.router_), cgi_handler_(other.cgi_handler_),
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
		delete current_response_;
		current_response_ = NULL;
		if (other.current_response_)
		{
			current_response_ = new HttpResponse(*other.current_response_);
		}
		reactor_ = other.reactor_;
		router_ = other.router_;
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

const std::string& Connection::getOutBuffer() const
{
	return out_buffer_;
}

int Connection::getFd() const
{
	return fd_;
}

ConnState Connection::getState() const
{
	return state_;
}

const HttpResponse* Connection::getCurrentResponse() const
{
	return current_response_;
}

void Connection::closeConnection()
{
	if (fd_ < 0)
	{
		return;
	}
	reactor_->unregisterHandler(fd_);
	close(fd_);
	fd_ = -1;
	state_ = CLOSED;
}

void Connection::handleReadEvent()
{
	static const std::size_t READ_CHUNK = 8192;
	char buffer[READ_CHUNK];
	ssize_t bytes_read = 0;

	if (fd_ < 0)
	{
		return;
	}
	bytes_read = recv(fd_, buffer, READ_CHUNK, 0);
	if (bytes_read <= 0)
	{
		closeConnection();
		return;
	}
	std::cout << bytes_read << '\n' << std::flush;
	in_buffer_.append(buffer, bytes_read);
	last_activity_ = time(NULL);
	processRequest();
}

void Connection::handleWriteEvent()
{
	ssize_t bytes_sent = 0;

	if (fd_ < 0)
	{
		return;
	}
	if (out_buffer_.empty())
	{
		return;
	}
	bytes_sent = send(fd_, &out_buffer_[0], out_buffer_.size(), 0);
	if (bytes_sent <= 0)
	{
		closeConnection();
		return;
	}
	out_buffer_.erase(0, static_cast<std::string::size_type>(bytes_sent));
	if (out_buffer_.empty())
	{
		reactor_->updateEvents(fd_, READ);
	}
}

void Connection::handleTimeout()
{
	closeConnection();
}

bool Connection::isTimedOut(time_t now) const
{
	if (fd_ < 0)
	{
		return false;
	}
	return now - last_activity_ >= server_config_.idle_timeout_;
}

bool Connection::wantsWrite() const
{
	return !out_buffer_.empty();
}

void Connection::queueOutput(const std::string& out_buffer)
{
	if (out_buffer.empty())
	{
		return;
	}
	out_buffer_ += out_buffer;
	if (fd_ >= 0)
	{
		reactor_->updateEvents(fd_, READ_WRITE);
	}
}

void Connection::processRequest()
{
	if (in_buffer_.empty())
	{
		return;
	}

	request_parser_.reset();
	request_parser_.feed(&in_buffer_[0], in_buffer_.size());

	if (request_parser_.isError())
	{
		const int status = request_parser_.errorStatus();
		HttpResponse* response = new HttpResponse();
		response->setStatus(
			status, status == 413 ? "Payload Too Large" : "Bad Request");
		delete current_response_;
		current_response_ = response;
		in_buffer_.clear();
		request_parser_.reset();
		sendResponse(*current_response_);
		return;
	}

	if (!request_parser_.isComplete())
	{
		return;
	}

	HttpResponse* response = new HttpResponse(
		router_.route(request_parser_.getRequest(), server_config_));
	delete current_response_;
	current_response_ = response;
	in_buffer_.erase(0, request_parser_.consumed());
	request_parser_.reset();
	sendResponse(*current_response_);
}

void Connection::sendResponse(HttpResponse& response)
{
	queueOutput(response.toString());
}
