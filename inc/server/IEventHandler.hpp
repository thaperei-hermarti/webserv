/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IEventHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:39 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:40 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_IEVENTHANDLER_HPP
#define WEBSERV_SERVER_IEVENTHANDLER_HPP

class IEventHandler
{
  public:
	virtual ~IEventHandler();
	virtual int getFd() const = 0;
	virtual void handleReadEvent() = 0;
	virtual void handleWriteEvent() = 0;
	virtual bool wantsWrite() const = 0;
	virtual void handleTimeout() = 0;
};

#endif
