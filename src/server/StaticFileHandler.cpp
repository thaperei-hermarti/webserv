/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.cpp                             :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/StaticFileHandler.hpp"

StaticFileHandler::StaticFileHandler()
{
}

StaticFileHandler::~StaticFileHandler()
{
}

StaticFileHandler::StaticFileHandler(const StaticFileHandler& other)
{
	(void) other;
}

StaticFileHandler& StaticFileHandler::operator=(const StaticFileHandler& other)
{
	(void) other;
	return *this;
}

HttpResponse StaticFileHandler::handle(HttpRequest& request,
									   LocationConfig& config)
{
	(void) request;
	(void) config;
	return HttpResponse();
}

HttpResponse StaticFileHandler::serveFile(const std::string& path)
{
	(void) path;
	return HttpResponse();
}

HttpResponse StaticFileHandler::listDirectory(const std::string& path)
{
	(void) path;
	return HttpResponse();
}
