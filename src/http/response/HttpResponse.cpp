/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:17 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:21:19 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/response/HttpResponse.hpp"

#include <sstream>

HttpResponse::HttpResponse() : status_code_(200)
{
}

HttpResponse::~HttpResponse()
{
}

HttpResponse::HttpResponse(const HttpResponse& other)
	: status_code_(other.status_code_), status_message_(other.status_message_),
	  headers_(other.headers_), body_(other.body_)
{
}

HttpResponse& HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		status_code_ = other.status_code_;
		status_message_ = other.status_message_;
		headers_ = other.headers_;
		body_ = other.body_;
	}
	return *this;
}

void HttpResponse::setStatus(int status_code, const std::string& status_message)
{
	status_code_ = status_code;
	status_message_ = status_message;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value)
{
	headers_[name] = value;
}

void HttpResponse::setBody(const std::string& body)
{
	body_ = body;
}

const std::string& HttpResponse::toString() const
{
	static const std::string EMPTY;
	(void) status_code_;
	(void) status_message_;
	(void) headers_;
	return EMPTY;
}
