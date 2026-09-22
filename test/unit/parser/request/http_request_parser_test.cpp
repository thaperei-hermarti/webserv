/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_request_parser_test.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:48 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:24:49 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/request/HttpRequestParser.hpp"

#include <cstring>
#include <gtest/gtest.h>

// Scenario: a complete, well-formed GET request arrives in a single feed() call.
//   Given : "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
//   When  : feed() consumes all bytes at once
//   Then  : parser reaches DONE and HttpRequest exposes method, uri, version and
//           the Host header that were parsed.
TEST(HttpRequestParserTest, ParsesSimpleGetRequest)
{
	HttpRequestParser parser(1024);

	const char* request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());

	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getVersion(), "HTTP/1.1");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
}

// Scenario: the same request arrives split across multiple feed() calls.
//   Given : the request line is delivered in two pieces
//   When  : feed() is called once per piece, then the rest of the request
//   Then  : the parser still reaches DONE with all fields correctly parsed
TEST(HttpRequestParserTest, RequestLineSplitAcrossFeeds)
{
	HttpRequestParser parser(1024);

	parser.feed("GET /index.html ", std::strlen("GET /index.html "));
	EXPECT_FALSE(parser.isComplete());

	parser.feed("HTTP/1.1\r\nHost: localhost\r\n\r\n",
				std::strlen("HTTP/1.1\r\nHost: localhost\r\n\r\n"));

	EXPECT_TRUE(parser.isComplete());
	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getVersion(), "HTTP/1.1");
	EXPECT_EQ(req.getHeader("Host"), "localhost");
}

// Scenario: a header line is split in the middle by feed() calls.
//   Given : "Host: localhost" arrives as "Ho" + "st: localhost"
//   When  : both pieces are fed
//   Then  : the parser still reaches DONE and finds the Host header
TEST(HttpRequestParserTest, HeadersSplitMidLine)
{
	HttpRequestParser parser(1024);

	parser.feed("GET / HTTP/1.1\r\nHo", std::strlen("GET / HTTP/1.1\r\nHo"));
	EXPECT_FALSE(parser.isComplete());

	parser.feed("st: localhost\r\n\r\n", std::strlen("st: localhost\r\n\r\n"));

	EXPECT_TRUE(parser.isComplete());
	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getHeader("Host"), "localhost");
}

// Scenario: the entire request is delivered one byte at a time.
//   Given : any valid request
//   When  : feed() is called with a single byte each time
//   Then  : the parser accumulates the data and reaches DONE
TEST(HttpRequestParserTest, ParsesByteByByte)
{
	HttpRequestParser parser(1024);

	const char* request = "GET /index.html HTTP/1.1\r\nHost: local\r\n\r\n";
	for (std::size_t i = 0; i < std::strlen(request); ++i)
	{
		parser.feed(request + i, 1);
		EXPECT_FALSE(parser.isError());
	}

	EXPECT_TRUE(parser.isComplete());
	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getMethod(), "GET");
	EXPECT_EQ(req.getUri(), "/index.html");
	EXPECT_EQ(req.getVersion(), "HTTP/1.1");
}

// Scenario: the request-target carries a query string.
//   Given : "GET /search?q=webserv HTTP/1.1"
//   When  : the request is parsed
//   Then  : the uri keeps the full target including the query string
TEST(HttpRequestParserTest, UriIncludesQueryString)
{
	HttpRequestParser parser(1024);

	const char* request =
		"GET /search?q=webserv HTTP/1.1\r\nHost: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_EQ(parser.getRequest().getUri(), "/search?q=webserv");
}

// Scenario: header field names are case-insensitive per RFC 7230.
//   Given : the Host header arrives as "HoSt: localhost"
//   When  : the request is parsed
//   Then  : both getHeader("Host") and getHeader("host") find its value
TEST(HttpRequestParserTest, HeaderNamesAreCaseInsensitive)
{
	HttpRequestParser parser(1024);

	const char* request = "GET / HTTP/1.1\r\nHoSt: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getHeader("Host"), "localhost");
	EXPECT_EQ(req.getHeader("host"), "localhost");
}

// Scenario: HTTP/1.1 requires the Host header.
//   Given : "GET / HTTP/1.1" followed by an empty header section
//   When  : headers are parsed
//   Then  : the parser reports an error (no Host -> 400)
TEST(HttpRequestParserTest, MissingHostIsErrorForHttp11)
{
	HttpRequestParser parser(1024);

	const char* request = "GET / HTTP/1.1\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}

// Scenario: a request with a Content-Length body.
//   Given : "POST /upload" declaring 5 body bytes and sending "hello"
//   When  : the whole request is fed
//   Then  : the parser reaches DONE and exposes the body
TEST(HttpRequestParserTest, ParsesContentLengthBody)
{
	HttpRequestParser parser(1024);

	const char* request = "POST /upload HTTP/1.1\r\nHost: localhost\r\n"
						  "Content-Length: 5\r\n\r\nhello";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_EQ(parser.getRequest().getBody(), "hello");
}

// Scenario: Content-Length of zero means there is no body.
//   Given : "Content-Length: 0" with headers closed
//   When  : the request is parsed
//   Then  : the parser reaches DONE with an empty body
TEST(HttpRequestParserTest, ZeroContentLengthMeansNoBody)
{
	HttpRequestParser parser(1024);

	const char* request =
		"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_EQ(parser.getRequest().getBody(), "");
}

// Scenario: Content-Length must be a valid decimal number.
//   Given : "Content-Length: abc"
//   When  : the request is parsed
//   Then  : the parser reports an error
TEST(HttpRequestParserTest, NonNumericContentLengthIsError)
{
	HttpRequestParser parser(1024);

	const char* request =
		"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
}

// Scenario: a body larger than max_body_size is rejected early.
//   Given : max_body_size = 4 and Content-Length: 5
//   When  : the request is parsed
//   Then  : the parser reports an error before reading any body bytes
TEST(HttpRequestParserTest, ContentLengthAboveMaxIsError)
{
	HttpRequestParser parser(4);

	const char* request =
		"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}

// Scenario: a body exactly at the limit is accepted (boundary check).
//   Given : max_body_size = 5 and Content-Length: 5
//   When  : the request is parsed
//   Then  : the parser reaches DONE with the body
TEST(HttpRequestParserTest, ContentLengthEqualToMaxIsAccepted)
{
	HttpRequestParser parser(5);

	const char* request =
		"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_EQ(parser.getRequest().getBody(), "hello");
}

// Scenario: a chunked body is decoded back to its plain content.
//   Given : Transfer-Encoding: chunked with one chunk "5\r\nhello" + terminator
//   When  : the request is parsed
//   Then  : the body is unchunked to "hello" and isChunked() is true
TEST(HttpRequestParserTest, ChunkedBodyIsUnchunked)
{
	HttpRequestParser parser(1024);

	const char* request = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n\r\n"
						  "5\r\nhello\r\n0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	HttpRequest& req = parser.getRequest();
	EXPECT_TRUE(req.isChunked());
	EXPECT_EQ(req.getBody(), "hello");
}

// Scenario: feeding zero bytes is a no-op.
//   Given : an empty feed call
//   When  : feed("", 0) is called
//   Then  : the parser stays idle: not complete, not in error
TEST(HttpRequestParserTest, EmptyFeedDoesNotChangeState)
{
	HttpRequestParser parser(1024);

	parser.feed("", 0);

	EXPECT_FALSE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
}

// Scenario: a request line with only two tokens has no HTTP version.
//   Given : "GET /" as the request line
//   When  : the request is parsed
//   Then  : the parser reports an error
TEST(HttpRequestParserTest, RequestLineMissingVersionIsError)
{
	HttpRequestParser parser(1024);

	const char* request = "GET /\r\nHost: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}
