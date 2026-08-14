/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:18:52 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:18:53 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_SESSIONMANAGER_HPP
#define WEBSERV_SERVER_SESSIONMANAGER_HPP

#include <map>
#include <string>

#include "server/Session.hpp"

class SessionManager
{
  public:
	SessionManager();
	~SessionManager();
	SessionManager(const SessionManager& other);
	SessionManager& operator=(const SessionManager& other);

	Session& createSession();
	Session* getSession(const std::string& id);
	void cleanupExpired();

  private:
	std::string generateSessionId() const;

	std::map<std::string, Session> sessions_;
};

#endif
