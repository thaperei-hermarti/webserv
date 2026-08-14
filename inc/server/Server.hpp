/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:18:24 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:18:25 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_SERVER_HPP
#define WEBSERV_SERVER_SERVER_HPP

#include <vector>

#include "cgi/CgiRegistry.hpp"
#include "config/ServerConfig.hpp"
#include "server/Reactor.hpp"
#include "server/SessionManager.hpp"

class Acceptor;

class Server
{
  public:
	explicit Server(const std::vector<ServerConfig>& configs);
	~Server();
	Server(const Server& other);
	Server& operator=(const Server& other);

	void init();
	void run();

  private:
	Reactor reactor_;
	std::vector<Acceptor*> acceptors_;
	std::vector<ServerConfig> configs_;
	CgiRegistry cgi_registry_;
	SessionManager session_manager_;
};

#endif
