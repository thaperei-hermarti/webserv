/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:16:46 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:48 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_PARSER_REQUEST_HTTPREQUESTPARSER_HPP
#define WEBSERV_PARSER_REQUEST_HTTPREQUESTPARSER_HPP

#include <cstddef>
#include <string>

#include "http/request/HttpRequest.hpp"

enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	DONE
};

class HttpRequestParser
{
  public:
	explicit HttpRequestParser(std::size_t max_body_size);
	~HttpRequestParser();
	HttpRequestParser(const HttpRequestParser& other);
	HttpRequestParser& operator=(const HttpRequestParser& other);

	void feed(const char* data, std::size_t len);
	bool isComplete() const;
	HttpRequest& getRequest();

  private:
	void parseRequestLine();
	void parseHeaders();
	void unchunkBody();

	ParseState state_;
	std::size_t max_body_size_;
	HttpRequest request_;
};

#endif
