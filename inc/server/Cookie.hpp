/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookie.hpp                                        :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#ifndef WEBSERV_SERVER_COOKIE_HPP
#define WEBSERV_SERVER_COOKIE_HPP

#include <map>
#include <string>

class Cookie
{
  public:
	Cookie();
	~Cookie();
	Cookie(const Cookie& other);
	Cookie& operator=(const Cookie& other);

	static Cookie parse(const std::string& header_value);
	const std::string& toSetCookieHeader() const;

  private:
	std::string name_;
	std::string value_;
	std::map<std::string, std::string> attributes_;
};

#endif
