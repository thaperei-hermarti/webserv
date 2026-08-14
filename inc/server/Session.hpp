/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:18:31 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:18:32 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_SESSION_HPP
#define WEBSERV_SERVER_SESSION_HPP

#include <ctime>
#include <map>
#include <string>

class Session
{
  public:
	Session();
	~Session();
	Session(const Session& other);
	Session& operator=(const Session& other);

	const std::string& get(const std::string& key) const;
	void set(const std::string& key, const std::string& value);
	bool isExpired() const;

  private:
	std::string id_;
	std::map<std::string, std::string> data_;
	time_t expiry_;
};

#endif
