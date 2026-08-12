/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                               :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "http/response/ResponseBuilder.hpp"

HttpResponse ResponseBuilder::buildErrorResponse(int code,
												 const ServerConfig& config)
{
	(void) code;
	(void) config;
	return HttpResponse();
}

HttpResponse ResponseBuilder::buildRedirect(const std::string& location,
											int code)
{
	(void) location;
	(void) code;
	return HttpResponse();
}

HttpResponse ResponseBuilder::buildFromFile(const std::string& path)
{
	(void) path;
	return HttpResponse();
}
