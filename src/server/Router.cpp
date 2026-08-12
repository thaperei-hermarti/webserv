/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                        :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "server/Router.hpp"

Router::Router()
{
}

Router::~Router()
{
}

Router::Router(const Router& other)
{
	(void) other;
}

Router& Router::operator=(const Router& other)
{
	(void) other;
	return *this;
}

HttpResponse Router::route(HttpRequest& request, ServerConfig& config)
{
	(void) request;
	(void) config;
	return HttpResponse();
}

HttpResponse Router::dispatchToHandler(LocationConfig& config,
									   HttpRequest& request)
{
	(void) config;
	(void) request;
	return HttpResponse();
}
