/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:16:12 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:12 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HTTP_RESPONSE_RESPONSEBUILDER_HPP
#define WEBSERV_HTTP_RESPONSE_RESPONSEBUILDER_HPP

#include <string>

#include "http/response/HttpResponse.hpp"

struct ServerConfig;

class ResponseBuilder
{
  public:
	static HttpResponse buildErrorResponse(int code,
										   const ServerConfig& config);
	static HttpResponse buildRedirect(const std::string& location, int code);
	static HttpResponse buildFromFile(const std::string& path);

  private:
	ResponseBuilder();
	ResponseBuilder(const ResponseBuilder& other);
	ResponseBuilder& operator=(const ResponseBuilder& other);
};

#endif
