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
#include "server/EventType.hpp"
#include "server/IEventHandler.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

Reactor::Reactor() : epoll_fd_(epoll_create(1))
{
	if (epoll_fd_ < 0)
	{
		std::cerr << "server:error:epoll_create:" << strerror(errno) << '\n';
		exit(EXIT_FAILURE);
	}
	events_.resize(MAX_EVENTS);
}

Reactor::~Reactor()
{
	if (epoll_fd_ >= 0)
	{
		close(epoll_fd_);
	}
}

void Reactor::registerHandler(IEventHandler* handler, EventType event_type)
{
	struct epoll_event ev;

	ev.events = event_type;
	ev.data.ptr = handler;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, handler->getFd(), &ev) == -1)
	{
		std::cerr << "server:error:epoll_ctl_add:" << strerror(errno) << '\n';
	}
	else
	{
		handlers_[handler->getFd()] = handler;
	}
}

void Reactor::unregisterHandler(int fd)
{
	if (handlers_.find(fd) == handlers_.end())
	{
		return;
	}
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1)
	{
		std::cerr << "server:error:epoll_ctl_del:" << strerror(errno) << '\n';
	}
	else
	{
		handlers_.erase(fd);
	}
}

void Reactor::updateEvents(int fd, EventType event_type)
{
	if (handlers_.find(fd) == handlers_.end())
	{
		return;
	}
	struct epoll_event ev;

	ev.events = event_type;
	ev.data.ptr = handlers_.at(fd);
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1)
	{
		std::cerr << "server:error:epoll_ctl_mod:" << strerror(errno) << '\n';
	}
}

void Reactor::run()
{
	while (true)
	{
		int active_fds = waitForEvents();
		if (active_fds < 0)
		{
			if (errno != EINTR)
			{
				std::cerr << "server:error:epoll_wait:" << strerror(errno)
						  << '\n';
				return;
			}
		}
		else
		{
			dispatch(active_fds);
		}
	}
}

int Reactor::waitForEvents()
{
	return epoll_wait(
		epoll_fd_, events_.data(), static_cast<int>(events_.size()), -1);
}

void Reactor::handleHangup(int fd)
{
	unregisterHandler(fd);
}

void Reactor::handleError(int fd)
{
	std::cerr << "server:error:EPOLLERR on fd " << fd << '\n';
	unregisterHandler(fd);
}

void Reactor::dispatch(int active_fds)
{
	for (int n = 0; n < active_fds; n++)
	{
		IEventHandler* handler =
			static_cast<IEventHandler*>(events_[n].data.ptr);
		if (static_cast<bool>(events_[n].events & EPOLLIN))
		{
			handler->handleReadEvent();
		}
		if (static_cast<bool>(events_[n].events & EPOLLOUT))
		{
			handler->handleWriteEvent();
		}
		if (static_cast<bool>(events_[n].events & EPOLLHUP))
		{
			handleHangup(handler->getFd());
		}
		if (static_cast<bool>(events_[n].events & EPOLLERR))
		{
			handleError(handler->getFd());
		}
	}
}
