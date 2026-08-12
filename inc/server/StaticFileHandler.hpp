/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.hpp                             :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
