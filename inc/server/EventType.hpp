/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventType.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:33 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:34 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_EVENTTYPE_HPP
#define WEBSERV_SERVER_EVENTTYPE_HPP

#include <cstdint>
#include <sys/epoll.h>

enum EventType: std::uint8_t
{
	READ = EPOLLIN,
	WRITE = EPOLLOUT,
	READ_WRITE = EPOLLIN | EPOLLOUT,
	TIMEOUT = 010,
};

#endif
