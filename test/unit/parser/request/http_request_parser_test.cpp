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

// Scenario: a header line without the required colon separator.
//   Given : "Host localhost" instead of "Host: localhost"
//   When  : the headers are parsed
//   Then  : the parser reports an error
TEST(HttpRequestParserTest, HeaderMissingColonIsError)
{
	HttpRequestParser parser(1024);

	const char* request = "GET / HTTP/1.1\r\nHost localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}

// Scenario: a chunk-size line that is not valid hexadecimal.
//   Given : Transfer-Encoding: chunked with chunk-size "g"
//   When  : the body is unchunked
//   Then  : the parser reports an error
TEST(HttpRequestParserTest, MalformedChunkSizeIsError)
{
	HttpRequestParser parser(1024);

	const char* request = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n\r\n"
						  "g\r\nhello\r\n0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}

// Scenario: a request line with an empty method token.
//   Given : a leading space before the method
//   When  : the request line is parsed
//   Then  : the parser reports an error
TEST(HttpRequestParserTest, EmptyMethodIsError)
{
	HttpRequestParser parser(1024);

	const char* request = " GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
}

// Scenario: a well-formed but non-listed method passes parsing (Option A).
//   Given : "PATCH / HTTP/1.1" with a valid Host header
//   When  : the request is parsed
//   Then  : the parser succeeds; method enforcement is left to routing
TEST(HttpRequestParserTest, UnknownMethodParsesFine)
{
	HttpRequestParser parser(1024);

	const char* request = "PATCH / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
	EXPECT_EQ(parser.getRequest().getMethod(), "PATCH");
}

// Scenario: the parser reports how many input bytes the completed request used.
//   Given : a POST with a 5-byte Content-Length body
//   When  : the whole request is fed and reaches DONE
//   Then  : consumed() equals the full request length, body included
TEST(HttpRequestParserTest, ConsumedReportsCompletedRequestLength)
{
	HttpRequestParser parser(1024);

	const char* request = "POST /upload HTTP/1.1\r\nHost: localhost\r\n"
						  "Content-Length: 5\r\n\r\nhello";
	parser.feed(request, std::strlen(request));

	ASSERT_TRUE(parser.isComplete());
	EXPECT_EQ(std::strlen(request), parser.consumed());
}

// Scenario: a parser is reused for the next request on the same connection.
//   Given : a completed GET, then reset()
//   When  : a different POST is fed
//   Then  : no state from the first request survives; the second parses fully
TEST(HttpRequestParserTest, ResetAllowsParsingASecondRequest)
{
	HttpRequestParser parser(1024);

	const char* first = "GET /first HTTP/1.1\r\nHost: localhost\r\n\r\n";
	parser.feed(first, std::strlen(first));
	ASSERT_TRUE(parser.isComplete());
	ASSERT_EQ(std::strlen(first), parser.consumed());

	parser.reset();
	EXPECT_FALSE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
	EXPECT_EQ(static_cast<std::size_t>(0), parser.consumed());

	const char* second = "POST /second HTTP/1.1\r\nHost: localhost\r\n"
						 "Content-Length: 4\r\n\r\nbody";
	parser.feed(second, std::strlen(second));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
	HttpRequest& req = parser.getRequest();
	EXPECT_EQ(req.getMethod(), "POST");
	EXPECT_EQ(req.getUri(), "/second");
	EXPECT_EQ(req.getBody(), "body");
	EXPECT_EQ(std::strlen(second), parser.consumed());
}

// Scenario: a Content-Length body above the configured limit is rejected.
//   Given : max_body_size = 4 and Content-Length: 5
//   When  : the headers are parsed
//   Then  : the parser is in error and reports 413 (Payload Too Large)
TEST(HttpRequestParserTest, OversizedContentLengthReportsPayloadTooLarge)
{
	HttpRequestParser parser(4);

	const char* request =
		"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
	EXPECT_EQ(413, parser.errorStatus());
}

// Scenario: a request arrives as headers first, body second.
//   Given : a POST with Content-Length: 5 whose headers are fed alone
//   When  : the body arrives in a second feed() call
//   Then  : consumed() accumulates both feeds instead of only the last one
TEST(HttpRequestParserTest, ConsumedAccumulatesAcrossFeeds)
{
	HttpRequestParser parser(1024);

	const char* headers = "POST /upload HTTP/1.1\r\nHost: localhost\r\n"
						  "Content-Length: 5\r\n\r\n";
	parser.feed(headers, std::strlen(headers));
	ASSERT_FALSE(parser.isComplete());
	EXPECT_EQ(std::strlen(headers), parser.consumed());

	parser.feed("hello", 5);

	ASSERT_TRUE(parser.isComplete());
	EXPECT_EQ(std::strlen(headers) + 5, parser.consumed());
}

// Scenario: a chunked body exactly at the limit is accepted.
//   Given : max_body_size = 5 and chunks "abc" + "de"
//   When  : the whole request is fed
//   Then  : the parser reaches DONE with the five decoded bytes
TEST(HttpRequestParserTest, ChunkedBodyEqualToMaxIsAccepted)
{
	HttpRequestParser parser(5);

	const char* request = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n\r\n"
						  "3\r\nabc\r\n2\r\nde\r\n0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
	EXPECT_EQ("abcde", parser.getRequest().getBody());
}

// Scenario: a single chunk above the limit is rejected.
//   Given : max_body_size = 4 and one chunk "hello"
//   When  : the whole request is fed
//   Then  : the parser errors with 413 and stores no body bytes
TEST(HttpRequestParserTest, ChunkedBodyAboveMaxReportsPayloadTooLarge)
{
	HttpRequestParser parser(4);

	const char* request = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n\r\n"
						  "5\r\nhello\r\n0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
	EXPECT_EQ(413, parser.errorStatus());
	EXPECT_EQ("", parser.getRequest().getBody());
}

// Scenario: multiple chunks share one cumulative body limit.
//   Given : max_body_size = 5 and chunks "abc" + "def"
//   When  : the whole request is fed
//   Then  : the second chunk trips 413 and only "abc" was decoded
TEST(HttpRequestParserTest, ChunkedBodyLimitAccumulatesAcrossChunks)
{
	HttpRequestParser parser(5);

	const char* request = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						  "Transfer-Encoding: chunked\r\n\r\n"
						  "3\r\nabc\r\n3\r\ndef\r\n0\r\n\r\n";
	parser.feed(request, std::strlen(request));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
	EXPECT_EQ(413, parser.errorStatus());
	EXPECT_EQ("abc", parser.getRequest().getBody());
}

// Scenario: the cumulative limit survives a feed() boundary at the limit.
//   Given : max_body_size = 5, first feed delivers "abc", second "de"
//   When  : both feeds are parsed
//   Then  : the parser reaches DONE with exactly five decoded bytes
TEST(HttpRequestParserTest, ChunkedBodyLimitAccumulatesAcrossFeeds)
{
	HttpRequestParser parser(5);

	const char* first = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						"Transfer-Encoding: chunked\r\n\r\n"
						"3\r\nabc\r\n";
	parser.feed(first, std::strlen(first));
	ASSERT_FALSE(parser.isComplete());
	ASSERT_FALSE(parser.isError());
	EXPECT_EQ("abc", parser.getRequest().getBody());

	const char* second = "2\r\nde\r\n0\r\n\r\n";
	parser.feed(second, std::strlen(second));

	EXPECT_TRUE(parser.isComplete());
	EXPECT_FALSE(parser.isError());
	EXPECT_EQ("abcde", parser.getRequest().getBody());
}

// Scenario: the cumulative limit is enforced across feed() calls.
//   Given : max_body_size = 5, first feed delivers "abc", second "def"
//   When  : both feeds are parsed
//   Then  : the second feed trips 413 and only "abc" remains decoded
TEST(HttpRequestParserTest,
	 ChunkedBodyAboveMaxAcrossFeedsReportsPayloadTooLarge)
{
	HttpRequestParser parser(5);

	const char* first = "POST / HTTP/1.1\r\nHost: localhost\r\n"
						"Transfer-Encoding: chunked\r\n\r\n"
						"3\r\nabc\r\n";
	parser.feed(first, std::strlen(first));
	ASSERT_FALSE(parser.isComplete());
	ASSERT_FALSE(parser.isError());

	const char* second = "3\r\ndef\r\n0\r\n\r\n";
	parser.feed(second, std::strlen(second));

	EXPECT_TRUE(parser.isError());
	EXPECT_FALSE(parser.isComplete());
	EXPECT_EQ(413, parser.errorStatus());
	EXPECT_EQ("abc", parser.getRequest().getBody());
}
