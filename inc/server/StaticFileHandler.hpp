/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:19:00 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:19:01 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_STATICFILEHANDLER_HPP
#define WEBSERV_SERVER_STATICFILEHANDLER_HPP

#include <string>

#include "server/IRequestHandler.hpp"

class StaticFileHandler : public IRequestHandler
{
  public:
	StaticFileHandler();
	~StaticFileHandler();
	StaticFileHandler(const StaticFileHandler& other);
	StaticFileHandler& operator=(const StaticFileHandler& other);

	HttpResponse handle(HttpRequest& request, LocationConfig& config);

  private:
	HttpResponse serveFile(const std::string& path);
	HttpResponse listDirectory(const std::string& path);
};

#endif
