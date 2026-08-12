/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Acceptor.cpp                                      :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/Acceptor.hpp"

Acceptor::Acceptor(const std::string& host,
				   int port,
				   ServerConfig& config,
				   Reactor& reactor)
	: listen_fd_(-1), config_(&config), reactor_(&reactor)
{
	(void) host;
	(void) port;
}

Acceptor::~Acceptor()
{
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
