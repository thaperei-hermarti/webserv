/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                 :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/UploadHandler.hpp"

UploadHandler::UploadHandler()
{
}

UploadHandler::~UploadHandler()
{
}

UploadHandler::UploadHandler(const UploadHandler& other)
{
	(void) other;
}

UploadHandler& UploadHandler::operator=(const UploadHandler& other)
{
	(void) other;
	return *this;
}

HttpResponse UploadHandler::handle(HttpRequest& request, LocationConfig& config)
{
	(void) request;
	(void) config;
	return HttpResponse();
}

void UploadHandler::storeFile(const std::string& store_path,
							  const std::string& body)
{
	(void) store_path;
	(void) body;
}
