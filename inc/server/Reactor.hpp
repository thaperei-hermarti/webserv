/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:18:08 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:18:09 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_REACTOR_HPP
#define WEBSERV_SERVER_REACTOR_HPP

#include <map>
#include <sys/epoll.h>
#include <vector>

#include "server/EventType.hpp"
#include "server/IEventHandler.hpp"

class Reactor
{
  public:
	Reactor();
	~Reactor();
	Reactor(const Reactor& other);
	Reactor& operator=(const Reactor& other);

	void registerHandler(IEventHandler* handler, EventType event_type);
	void unregisterHandler(int fd);
	void updateEvents(int fd, EventType event_type);
	void run();

  private:
	int waitForEvents();
	void dispatch();

	std::map<int, IEventHandler*> handlers_;
	int epoll_fd_;
	std::vector<struct epoll_event> events_;
};

#endif
