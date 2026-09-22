/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:44 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:21:45 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/request/HttpRequestParser.hpp"

HttpRequestParser::HttpRequestParser(std::size_t max_body_size)
	: state_(REQUEST_LINE), max_body_size_(max_body_size), cursor_(0)
{
}

HttpRequestParser::~HttpRequestParser()
{
}

HttpRequestParser::HttpRequestParser(const HttpRequestParser& other)
	: state_(other.state_), max_body_size_(other.max_body_size_),
	  buffer_(other.buffer_), cursor_(other.cursor_), request_(other.request_)
{
}

HttpRequestParser& HttpRequestParser::operator=(const HttpRequestParser& other)
{
	if (this != &other)
	{
		state_ = other.state_;
		max_body_size_ = other.max_body_size_;
		buffer_ = other.buffer_;
		cursor_ = other.cursor_;
		request_ = other.request_;
	}
	return *this;
}

void HttpRequestParser::feed(const char* data, std::size_t len)
{
	if (len > 0)
	{
		buffer_.append(data, len);
	}
}

bool HttpRequestParser::isComplete() const
{
	return state_ == DONE;
}

bool HttpRequestParser::isError() const
{
	return state_ == ERROR;
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