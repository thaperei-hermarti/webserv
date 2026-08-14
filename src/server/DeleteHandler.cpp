/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:22:24 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:22:25 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/DeleteHandler.hpp"

DeleteHandler::DeleteHandler()
{
}

DeleteHandler::~DeleteHandler()
{
}

DeleteHandler::DeleteHandler(const DeleteHandler& other)
{
	(void) other;
}

DeleteHandler& DeleteHandler::operator=(const DeleteHandler& other)
{
	(void) other;
	return *this;
}

HttpResponse DeleteHandler::handle(HttpRequest& request, LocationConfig& config)
{
	(void) request;
	(void) config;
	return HttpResponse();
}
