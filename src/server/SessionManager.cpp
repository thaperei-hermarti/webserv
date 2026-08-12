/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/SessionManager.hpp"

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

SessionManager::SessionManager(const SessionManager& other)
	: sessions_(other.sessions_)
{
}

SessionManager& SessionManager::operator=(const SessionManager& other)
{
	if (this != &other)
	{
		sessions_ = other.sessions_;
	}
	return *this;
}

Session& SessionManager::createSession()
{
	return sessions_[generateSessionId()];
}

Session* SessionManager::getSession(const std::string& id)
{
	std::map<std::string, Session>::iterator it = sessions_.find(id);
	return it != sessions_.end() ? &(it->second) : NULL;
}

void SessionManager::cleanupExpired()
{
}

std::string SessionManager::generateSessionId() const
{
	return std::string();
}
