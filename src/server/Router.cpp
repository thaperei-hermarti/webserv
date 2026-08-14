/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:52 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:52 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
