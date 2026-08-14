/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:23:14 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:23:15 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
