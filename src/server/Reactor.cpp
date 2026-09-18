/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:45 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:46 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Reactor.hpp"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

Reactor::Reactor() : epoll_fd_(epoll_create(1)) {
	if (epoll_fd_ < 0) {
		exit(EXIT_FAILURE);
	}
}

Reactor::~Reactor()
{
	if (epoll_fd_ >= 0)
	{
		close(epoll_fd_);
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
	struct epoll_event ev;

	ev.events = event_type;
	ev.data.ptr = handler;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler.getFd(), &ev) == -1) {
		std::cerr << "server:error:epoll_ctl:" << strerror(errno) << std::endl;
	}
	events_.push_back()
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
	while (true) {
		int nfds = epoll_wait(epoll_fd_, events_.data(), events_.size(), -1);
		for (int n = 0; n < nfds; ++n) {
			IEventHandler	*handler = static_cast<IEventHandler *>(events_[n].data.ptr);
			handler.
		}
	}
	return 0;
}

void Reactor::dispatch()
{
}
