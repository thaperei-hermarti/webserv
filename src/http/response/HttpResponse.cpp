/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:17 by hermarti          #+#    #+#             */
/*   Updated: 2026/11/18 20:15:15 by thaperei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/response/HttpResponse.hpp"

#include <cctype>
#include <sstream>
#include <utility>

static std::string toLower(const std::string& value)
{
	std::string lower = value;
	for (std::size_t i = 0; i < lower.size(); ++i)
	{
		lower[i] = static_cast<char>(
			std::tolower(static_cast<unsigned char>(lower[i])));
	}
	return lower;
}

static bool containsLineBreak(const std::string& value)
{
	return value.find('\r') != std::string::npos
		|| value.find('\n') != std::string::npos;
}

HttpResponse::HttpResponse() : status_code_(200), status_message_("OK")
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
		serialized_response_.clear();
	}
	return *this;
}

void HttpResponse::setStatus(int status_code, const std::string& status_message)
{
	status_code_ = status_code;
	status_message_ = status_message;
	serialized_response_.clear();
}

int HttpResponse::getStatusCode() const
{
	return status_code_;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value)
{
	const std::string lower_name = toLower(name);
	for (std::map<std::string, std::string>::iterator it = headers_.begin();
		 it != headers_.end(); ++it)
	{
		if (toLower(it->first) == lower_name)
		{
			headers_.erase(it);
			break;
		}
	}
	headers_[name] = value;
	serialized_response_.clear();
}

void HttpResponse::setBody(const std::string& body)
{
	body_ = body;
	serialized_response_.clear();
}

const std::string& HttpResponse::toString() const
{
	std::map<std::string, std::pair<std::string, std::string> > sorted_headers;
	for (std::map<std::string, std::string>::const_iterator it =
			 headers_.begin();
		 it != headers_.end(); ++it)
	{
		if (!containsLineBreak(it->first) && !containsLineBreak(it->second))
			sorted_headers[toLower(it->first)] =
				std::make_pair(it->first, it->second);
	}

	const bool informational = status_code_ >= 100 && status_code_ < 200;
	const bool no_content = status_code_ == 204;
	const bool not_modified = status_code_ == 304;
	const bool body_allowed =
		!informational && !no_content && !not_modified;
	std::map<std::string, std::pair<std::string, std::string> >::const_iterator
		content_length = sorted_headers.find("content-length");

	std::ostringstream response;
	response << "HTTP/1.1 " << status_code_;
	if (!status_message_.empty())
		response << ' ' << status_message_;
	response << "\r\n";

	if (body_allowed)
	{
		std::string length;
		if (content_length != sorted_headers.end() && body_.empty())
			length = content_length->second.second;
		else
		{
			std::ostringstream length_stream;
			length_stream << body_.size();
			length = length_stream.str();
		}
		response << (content_length != sorted_headers.end()
						 ? content_length->second.first
						 : "Content-Length")
				 << ": " << length << "\r\n";
	}
	else if (not_modified && content_length != sorted_headers.end())
	{
		response << content_length->second.first << ": "
				 << content_length->second.second << "\r\n";
	}

	for (std::map<std::string, std::pair<std::string, std::string> >::
			 const_iterator it = sorted_headers.begin();
		 it != sorted_headers.end(); ++it)
	{
		if (it->first != "content-length")
			response << it->second.first << ": " << it->second.second
					 << "\r\n";
	}
	response << "\r\n";
	if (body_allowed)
		response << body_;

	serialized_response_ = response.str();
	return serialized_response_;
}
