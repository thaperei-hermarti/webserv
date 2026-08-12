/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                             :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "parser/request/HttpRequestParser.hpp"

HttpRequestParser::HttpRequestParser(std::size_t max_body_size)
	: state_(REQUEST_LINE), max_body_size_(max_body_size)
{
}

HttpRequestParser::~HttpRequestParser()
{
}

HttpRequestParser::HttpRequestParser(const HttpRequestParser& other)
	: state_(other.state_), max_body_size_(other.max_body_size_),
	  request_(other.request_)
{
}

HttpRequestParser& HttpRequestParser::operator=(const HttpRequestParser& other)
{
	if (this != &other)
	{
		state_ = other.state_;
		max_body_size_ = other.max_body_size_;
		request_ = other.request_;
	}
	return *this;
}

void HttpRequestParser::feed(const char* data, std::size_t len)
{
	(void) data;
	(void) len;
}

bool HttpRequestParser::isComplete() const
{
	return state_ == DONE;
}

HttpRequest& HttpRequestParser::getRequest()
{
	return request_;
}

void HttpRequestParser::parseRequestLine()
{
}

void HttpRequestParser::parseHeaders()
{
}

void HttpRequestParser::unchunkBody()
{
}
