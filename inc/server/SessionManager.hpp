/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
