/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Acceptor.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:16:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:56 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_ACCEPTOR_HPP
#define WEBSERV_SERVER_ACCEPTOR_HPP

#include <string>

#include "server/IEventHandler.hpp"

class Reactor;
struct ServerConfig;

class Acceptor : public IEventHandler
{
  public:
	Acceptor(const std::string& host,
			 int port,
			 ServerConfig& config,
			 Reactor& reactor);
	~Acceptor();
	Acceptor(const Acceptor& other);
	Acceptor& operator=(const Acceptor& other);

	int getFd() const;
	void handleReadEvent();
	void handleWriteEvent();
	void handleTimeout();

  private:
	void acceptNewClient();

	int listen_fd_;
	ServerConfig* config_;
	Reactor* reactor_;
};

#endif
