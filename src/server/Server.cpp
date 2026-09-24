/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:58 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:58 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Server.hpp"

#include "server/Acceptor.hpp"

#include <csignal>

Server::Server(const std::vector<ServerConfig>& configs) : configs_(configs)
{
}

Server::~Server()
{
	for (std::vector<Acceptor*>::const_iterator it = acceptors_.begin();
		 it != acceptors_.end();
		 ++it)
	{
		delete *it;
	}
}

Server::Server(const Server& other)
	: acceptors_(other.acceptors_), configs_(other.configs_),
	  cgi_registry_(other.cgi_registry_),
	  session_manager_(other.session_manager_)
{
}

Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		acceptors_ = other.acceptors_;
		configs_ = other.configs_;
		cgi_registry_ = other.cgi_registry_;
		session_manager_ = other.session_manager_;
	}
	return *this;
}

void Server::ignoreSigPipe()
{
	signal(SIGPIPE, SIG_IGN);
}

void Server::init()
{
	ignoreSigPipe();
}

void Server::run()
{
}
