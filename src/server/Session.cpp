/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                       :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/Session.hpp"

Session::Session() : expiry_(0)
{
}

Session::~Session()
{
}

Session::Session(const Session& other)
	: id_(other.id_), data_(other.data_), expiry_(other.expiry_)
{
}

Session& Session::operator=(const Session& other)
{
	if (this != &other)
	{
		id_ = other.id_;
		data_ = other.data_;
		expiry_ = other.expiry_;
	}
	return *this;
}

const std::string& Session::get(const std::string& key) const
{
	static const std::string EMPTY;
	std::map<std::string, std::string>::const_iterator it = data_.find(key);
	return it != data_.end() ? it->second : EMPTY;
}

void Session::set(const std::string& key, const std::string& value)
{
	data_[key] = value;
}

bool Session::isExpired() const
{
	return time(NULL) >= expiry_;
}
