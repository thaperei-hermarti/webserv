/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                               :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

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
