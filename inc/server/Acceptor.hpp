/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Acceptor.hpp                                      :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
