/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRequestHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:17:47 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:17:48 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_IREQUESTHANDLER_HPP
#define WEBSERV_SERVER_IREQUESTHANDLER_HPP

#include "http/response/HttpResponse.hpp"

class HttpRequest;
struct LocationConfig;

class IRequestHandler
{
  public:
	virtual ~IRequestHandler();
	virtual HttpResponse handle(HttpRequest& request,
								LocationConfig& config) = 0;
};

#endif
