/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                   :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "http/request/HttpRequest.hpp"

HttpRequest::HttpRequest() : is_chunked_(false)
{
}

HttpRequest::~HttpRequest()
{
}

HttpRequest::HttpRequest(const HttpRequest& other)
	: method_(other.method_), uri_(other.uri_), version_(other.version_),
	  headers_(other.headers_), body_(other.body_),
	  is_chunked_(other.is_chunked_)
{
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		method_ = other.method_;
		uri_ = other.uri_;
		version_ = other.version_;
		headers_ = other.headers_;
		body_ = other.body_;
		is_chunked_ = other.is_chunked_;
	}
	return *this;
}

const std::string& HttpRequest::getMethod() const
{
	return method_;
}

const std::string& HttpRequest::getUri() const
{
	return uri_;
}

const std::string& HttpRequest::getVersion() const
{
	return version_;
}

const std::string& HttpRequest::getHeader(const std::string& name) const
{
	static const std::string EMPTY;
	std::map<std::string, std::string>::const_iterator it = headers_.find(name);
	return it != headers_.end() ? it->second : EMPTY;
}

const std::string& HttpRequest::getBody() const
{
	return body_;
}

bool HttpRequest::isChunked() const
{
	return is_chunked_;
}
