/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:16:06 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:06 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HTTP_RESPONSE_HTTPRESPONSE_HPP
#define WEBSERV_HTTP_RESPONSE_HTTPRESPONSE_HPP

#include <map>
#include <string>

class HttpResponse
{
  public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse& other);
	HttpResponse& operator=(const HttpResponse& other);

	void setStatus(int status_code, const std::string& status_message);
	void setHeader(const std::string& name, const std::string& value);
	void setBody(const std::string& body);
	const std::string& toString() const;

  private:
	int status_code_;
	std::string status_message_;
	std::map<std::string, std::string> headers_;
	std::string body_;
};

#endif
