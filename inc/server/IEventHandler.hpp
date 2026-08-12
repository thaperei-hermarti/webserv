/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IEventHandler.hpp                                 :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
