/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:15:59 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HTTP_REQUEST_HTTPREQUEST_HPP
#define WEBSERV_HTTP_REQUEST_HTTPREQUEST_HPP

#include <map>
#include <string>

class HttpRequest
{
  public:
	HttpRequest();
	~HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);

	const std::string& getMethod() const;
	const std::string& getUri() const;
	const std::string& getVersion() const;
	const std::string& getHeader(const std::string& name) const;
	const std::string& getBody() const;
	bool isChunked() const;

  private:
	std::string method_;
	std::string uri_;
	std::string version_;
	std::map<std::string, std::string> headers_;
	std::string body_;
	bool is_chunked_;
};

#endif
