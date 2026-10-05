/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Acceptor.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:53 by hermarti          #+#    #+#             */
/*   Updated: 2026/10/05 18:48:23 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <server/EventType.hpp>
#include "server/Reactor.hpp"
#include "server/Acceptor.hpp"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Acceptor::Acceptor(const std::string& host,
				   int port,
				   ServerConfig& config,
				   Reactor& reactor)
	: listen_fd_(-1), config_(&config), reactor_(&reactor)
{
	std::ostringstream os;
	struct addrinfo hints;
	struct addrinfo* result = NULL;
	int rc = 0;
	int option = 1;
	int flags = 0;

	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	os << port;
	rc = getaddrinfo(host.c_str(), os.str().c_str(), &hints, &result);
	if (rc != 0)
	{
		std::cerr << "server:error:getaddrinfo:" << gai_strerror(rc) << '\n';
		return;
	}
	for (struct addrinfo* ai = result; ai != NULL; ai = ai->ai_next)
	{

		listen_fd_ = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (listen_fd_ < 0)
		{
			std::cerr << "server:error:socket:" << strerror(errno) << '\n';
			continue;
		}
		if (setsockopt(listen_fd_,
					   SOL_SOCKET,
					   SO_REUSEADDR,
					   &option,
					   sizeof(option)) != 0)
		{
			std::cerr << "server:error:setsockopt:" << strerror(errno) << '\n';
			close(listen_fd_);
			listen_fd_ = -1;
			continue;
		}
		if (bind(listen_fd_, ai->ai_addr, ai->ai_addrlen) == 0)
		{
			break;
		}
		std::cerr << "server:error:bind:" << strerror(errno) << '\n';
		close(listen_fd_);
		listen_fd_ = -1;
	}
	freeaddrinfo(result);
	if (listen_fd_ < 0)
	{
		std::cerr << "server:error:bind:unable to bind " << host << ':' << port
				  << '\n';
		return;
	}
	if (listen(listen_fd_, SOMAXCONN) != 0)
	{
		std::cerr << "server:error:listen:" << strerror(errno) << '\n';
		close(listen_fd_);
		listen_fd_ = -1;
		return;
	}
	flags = fcntl(listen_fd_, F_GETFL, 0);
	if (flags == -1 || fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "server:error:fcntl:" << strerror(errno) << '\n';
		close(listen_fd_);
		listen_fd_ = -1;
		return;
	}
	reactor_->registerHandler(this, READ);
}

Acceptor::~Acceptor()
{
	if (listen_fd_ >= 0)
	{
		close(listen_fd_);
	}
	listen_fd_ = -1;
}

Acceptor::Acceptor(const Acceptor& other)
	: listen_fd_(other.listen_fd_), config_(other.config_),
	  reactor_(other.reactor_)
{
}

Acceptor& Acceptor::operator=(const Acceptor& other)
{
	if (this != &other)
	{
		listen_fd_ = other.listen_fd_;
		config_ = other.config_;
		reactor_ = other.reactor_;
	}
	return *this;
}

int Acceptor::getFd() const
{
	return listen_fd_;
}

void Acceptor::handleReadEvent()
{
}

void Acceptor::handleWriteEvent()
{
}

void Acceptor::handleTimeout()
{
}

void Acceptor::acceptNewClient()
{
}
