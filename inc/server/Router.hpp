/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:18:17 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:18:18 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_SERVER_ROUTER_HPP
#define WEBSERV_SERVER_ROUTER_HPP

#include "http/response/HttpResponse.hpp"

class HttpRequest;
struct LocationConfig;
struct ServerConfig;

class Router
{
  public:
	Router();
	~Router();
	Router(const Router& other);
	Router& operator=(const Router& other);

	HttpResponse route(HttpRequest& request, ServerConfig& config);

  private:
	HttpResponse dispatchToHandler(LocationConfig& config,
								   HttpRequest& request);
};

#endif
