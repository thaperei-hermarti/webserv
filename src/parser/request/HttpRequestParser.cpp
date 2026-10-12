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

// trim to " Host " => "Host"

static std::string trim(const std::string& str)
{
	std::size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return "";
	std::size_t last = str.find_last_not_of(" \t");
	return str.substr(first, last - first + 1);
}

static std::size_t hexValue(char c)
{
	if (c >= '0' && c <= '9')
	{
		return static_cast<std::size_t>(c - '0');
	}
	if (c >= 'a' && c <= 'f')
	{
		return static_cast<std::size_t>(c - 'a' + 10);
	}
	return static_cast<std::size_t>(c - 'A' + 10);
}

HttpRequestParser::HttpRequestParser(std::size_t max_body_size)
	: state_(REQUEST_LINE), max_body_size_(max_body_size), cursor_(0),
	body_remaining_(0), chunked_(false)
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

/*  
 ** acumulation in buffer_
 ** cursor_ to already consumation
 ** erase(0, cursor) in the end of feed
*/

void HttpRequestParser::feed(const char* data, std::size_t len)
{
	if (len == 0 || state_ == ERROR || state_ == DONE)
		return;
	buffer_.append(data, len);
	while (state_ != ERROR && state_ != DONE)
	{
		std::size_t before = cursor_;
		switch (state_)
		{
			case REQUEST_LINE:
				parseRequestLine();
				break;
			case HEADERS:
				parseHeaders();
				break;
			case BODY:
				if (chunked_)
					unchunkBody();
				else
					parseBody();
				break;
			default:
				break;
		}
		if (cursor_ == before)
			break;
	}
	if (cursor_ > 0)
	{
		buffer_.erase(0, cursor_);
		cursor_  = 0;
	}
}

bool HttpRequestParser::isComplete() const
{
	return state_ == DONE;
}

HttpRequest& HttpRequestParser::getRequest()
{
	return request_;
}

// 3 topkens; Extra spcace (void method) or missing token => ERROr;

void HttpRequestParser::parseRequestLine()
{
	std::size_t end = buffer_.find("\r\n", cursor_);
	if (end == std::string::npos)
		return ;
	std::string line = buffer_.substr(cursor_, end - cursor_);
	std::size_t first = line.find(' ');
	std::size_t second = line.find(' ', first + 1);
	if (first == std::string::npos || second == std::string::npos ||
			line.find(' ', second + 1) != std::string::npos)
	{
		state_ = ERROR;
		return;
	}
	request_.setMethod(line.substr(0, first));
	request_.setUri(line.substr(first + 1, second - first - 1));
	request_.setVersion(line.substr(second + 1));
	cursor_ = end + 2;
	state_ = HEADERS;
}

void HttpRequestParser::parseHeaders()
{
	std::size_t end = buffer_.find("\r\n\r\n", cursor_);
	if (end == std::string::npos)
		return ;
	std::size_t pos = cursor_;
	while (pos < end)
	{
		std::size_t line_end = buffer_.find("\r\n", pos);
		if (line_end == std::string::npos || line_end > end)
			break ;
		std::string line = buffer_.substr(pos, line_end - pos);
		std::size_t colon = line.find(':');
		if (colon == std::string::npos)
		{
			state_ = ERROR;
			return;
		}
		request_.setHeader(trim(line.substr(0, colon)),
					trim(line.substr(colon + 1)));
		pos = line_end + 2;
	}
	cursor_ = end + 4;
	if (request_.getVersion() == "HTTP/1.1" &&
			request_.getHeader("host").empty())
	{
		state_ = ERROR;
		return;
	}
	if (request_.getHeaders().count("content-length") > 0)
	{
		const std::string& value = request_.getHeader("content-length");
		if (value.empty() ||
			value.find_first_not_of("0123456789") != std::string::npos)
		{
			state_ = ERROR;
			return;
		}
		std::size_t length = 0;
		for (std::size_t i = 0; i < value.size(); ++i)
			length = length * 10 + static_cast<std::size_t>(value[i] - '0');
		if (length > max_body_size_)
		{
			state_ = ERROR;
			return;
		}
		body_remaining_ = length;
		if (length == 0)
			state_ = DONE;
		else
			state_ = BODY;
		return;
	}
	if (request_.getHeader("transfer-encoding") == "chunked")
	{
		chunked_ = true;
		request_.setChunked(true);
		state_ = BODY;
		return;
	}
	state_ = DONE;
}

void HttpRequestParser::parseBody()
{
	if (buffer_.size() - cursor_ < body_remaining_)
		return;
	request_.setBody(buffer_.substr(cursor_, body_remaining_));
	cursor_ +=  body_remaining_;
	state_ = DONE;
}

void HttpRequestParser::unchunkBody()
{
	std::size_t line_end = buffer_.find("\r\n", cursor_);
	if (line_end == std::string::npos)
		return ;
	std::string size_line = buffer_.substr(cursor_, line_end - cursor_);
	std::size_t size = 0;
	std::size_t i = 0;
	for (; i < size_line.size(); ++i)
	{
		char c = size_line[i];
		if (c == ';')
			break;
		if (!std::isxdigit(static_cast<unsigned char>(c)))
		{
			state_ = ERROR;
			return;
		}
		size = size * 16 + hexValue(c);
	}
	if (i == 0)
	{
		state_ = ERROR;
		return;
	}
	cursor_ = line_end + 2;
	if (size == 0)
	{
		if (buffer_.size() - cursor_ < 2)
			return;
		if (buffer_[cursor_] != '\r' || buffer_[cursor_ + 1] != '\n')
		{
			state_ = ERROR;
			return;
		}
		cursor_ += 2;
		state_ = DONE;
		return;
	}
	if (buffer_.size() - cursor_ < size + 2)
		return;
	request_.setBody(request_.getBody() + buffer_.substr(cursor_, size));
	cursor_ += size;
	if (buffer_[cursor_] != '\r' || buffer_[cursor_ + 1] != '\n')
	{
		state_ = ERROR;
		return;
	}
	cursor_ += 2;
}
