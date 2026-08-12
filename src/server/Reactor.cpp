/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                       :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/Reactor.hpp"

#include <sys/epoll.h>
#include <unistd.h>

Reactor::Reactor() : epoll_fd_(epoll_create1(0))
{
}

Reactor::~Reactor()
{
	if (epoll_fd_ >= 0)
	{
		::close(epoll_fd_);
	}
}

Reactor::Reactor(const Reactor& other)
	: handlers_(other.handlers_), epoll_fd_(other.epoll_fd_),
	  events_(other.events_)
{
}

Reactor& Reactor::operator=(const Reactor& other)
{
	if (this != &other)
	{
		handlers_ = other.handlers_;
		epoll_fd_ = other.epoll_fd_;
		events_ = other.events_;
	}
	return *this;
}

void Reactor::registerHandler(IEventHandler* handler, EventType event_type)
{
	(void) handler;
	(void) event_type;
}

void Reactor::unregisterHandler(int fd)
{
	(void) fd;
}

void Reactor::updateEvents(int fd, EventType event_type)
{
	(void) fd;
	(void) event_type;
}

void Reactor::run()
{
}

int Reactor::waitForEvents()
{
	return 0;
}

void Reactor::dispatch()
{
}
