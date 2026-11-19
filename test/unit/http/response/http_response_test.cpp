/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_response_test.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:27 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:24:29 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/response/HttpResponse.hpp"

#include <gtest/gtest.h>
#include <string>

// Scenario: a freshly constructed response has not been configured yet.
//   Given : a default HttpResponse (200, no headers, empty body)
//   When  : toString() serializes it
//   Then  : the status line says 200 OK and Content-Length is 0
TEST(HttpResponseTest, DefaultResponseHasStatusLineAndZeroContentLength)
{
	HttpResponse response;

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
}

// Scenario: the handler chooses an explicit status code and reason phrase.
//   Given : setStatus(404, "Not Found") on a default response
//   When  : toString() serializes it
//   Then  : the status line reflects code and message
TEST(HttpResponseTest, SetStatusWritesStatusLine)
{
	HttpResponse response;
	response.setStatus(404, "Not Found");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 404 Not Found\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
}

// Scenario: a single header is added to the response.
//   Given : setHeader("Content-Type", "text/html")
//   When  : toString() serializes it
//   Then  : the header line appears before the blank line
TEST(HttpResponseTest, SetHeaderWritesHeaderLine)
{
	HttpResponse response;
	response.setHeader("Content-Type", "text/html");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "Content-Type: text/html\r\n"
			  "\r\n");
}

// Scenario: the response carries a body.
//   Given : setBody("hello world")
//   When  : toString() serializes it
//   Then  : Content-Length matches the body and the bytes follow the blank line
TEST(HttpResponseTest, BodyIsWrittenAfterBlankLine)
{
	HttpResponse response;
	response.setBody("hello world");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 11\r\n"
			  "\r\n"
			  "hello world");
}

// Scenario: headers are added out of alphabetical order.
//   Given : X-Zeta, X-Alpha and Content-Type set in that order
//   When  : toString() serializes it
//   Then  : headers are emitted ordered by their lowercase name
TEST(HttpResponseTest, HeadersAreWrittenInAlphabeticalOrder)
{
	HttpResponse response;
	response.setHeader("X-Zeta", "z");
	response.setHeader("X-Alpha", "a");
	response.setHeader("Content-Type", "text/plain");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "Content-Type: text/plain\r\n"
			  "X-Alpha: a\r\n"
			  "X-Zeta: z\r\n"
			  "\r\n");
}

// Scenario: the same header is set twice.
//   Given : setHeader("X-Trace", "1") then setHeader("X-Trace", "2")
//   When  : toString() serializes it
//   Then  : only one line remains and it carries the last value
TEST(HttpResponseTest, RepeatedHeaderKeepsLastValue)
{
	HttpResponse response;
	response.setHeader("X-Trace", "1");
	response.setHeader("X-Trace", "2");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "X-Trace: 2\r\n"
			  "\r\n");
}

// Scenario: an explicit Content-Length is set with an empty body (HEAD case).
//   Given : setHeader("Content-Length", "42") and no body
//   When  : toString() serializes it
//   Then  : the explicit value is preserved (the body would be a resource size)
TEST(HttpResponseTest, ExplicitContentLengthIsPreservedWhenBodyIsEmpty)
{
	HttpResponse response;
	response.setHeader("Content-Length", "42");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 42\r\n"
			  "\r\n");
}

// Scenario: the caller sets Content-Length with a different case.
//   Given : setHeader("content-length", "5") and a 5-byte body
//   When  : toString() serializes it
//   Then  : the header is treated as present (no injected duplicate)
TEST(HttpResponseTest, ContentLengthDetectionIsCaseInsensitive)
{
	HttpResponse response;
	response.setHeader("content-length", "5");
	response.setBody("hello");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "content-length: 5\r\n"
			  "\r\n"
			  "hello");
}

// Scenario: the body itself contains CRLF bytes.
//   Given : setBody("line1\r\nline2")
//   When  : toString() serializes it
//   Then  : the body bytes are appended verbatim
TEST(HttpResponseTest, BodyWithCrlfIsWrittenVerbatim)
{
	HttpResponse response;
	response.setBody("line1\r\nline2");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 12\r\n"
			  "\r\n"
			  "line1\r\nline2");
}

// Scenario: a configured response is copied through the copy constructor.
//   Given : a response with status, header and body
//   When  : a copy is made and serialized
//   Then  : the copy produces the same bytes
TEST(HttpResponseTest, CopyConstructorPreservesSerialization)
{
	HttpResponse original;
	original.setStatus(201, "Created");
	original.setHeader("Content-Type", "application/json");
	original.setBody("{\"ok\":true}");

	HttpResponse copy(original);

	EXPECT_EQ(copy.toString(),
			  "HTTP/1.1 201 Created\r\n"
			  "Content-Length: 11\r\n"
			  "Content-Type: application/json\r\n"
			  "\r\n"
			  "{\"ok\":true}");
}

// Scenario: a response is assigned over one that had its own state.
//   Given : a configured source and a target with stale header
//   When  : operator= copies the source into the target
//   Then  : the target serializes exactly like the source
TEST(HttpResponseTest, AssignmentPreservesSerialization)
{
	HttpResponse source;
	source.setStatus(500, "Internal Server Error");
	source.setBody("boom");

	HttpResponse target;
	target.setHeader("X-Stale", "1");
	target = source;

	EXPECT_EQ(target.toString(),
			  "HTTP/1.1 500 Internal Server Error\r\n"
			  "Content-Length: 4\r\n"
			  "\r\n"
			  "boom");
}

// Scenario: an explicit Content-Length does not match the body.
//   Given : setHeader("Content-Length", "42") and a 5-byte body
//   When  : toString() serializes it
//   Then  : the value is recomputed from the body, keeping the header name case
TEST(HttpResponseTest, MismatchedContentLengthIsRecomputedFromBody)
{
	HttpResponse response;
	response.setHeader("Content-Length", "42");
	response.setBody("hello");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 5\r\n"
			  "\r\n"
			  "hello");
}

// Scenario: toString() is called before and after the body is configured.
//   Given : a default response serialized, then setBody("abc") and serialized
//   When  : toString() runs twice
//   Then  : the second call reflects the new Content-Length (no cached header)
TEST(HttpResponseTest, ToStringTwiceReflectsLaterBodyChange)
{
	HttpResponse response;
	const std::string before = response.toString();

	response.setBody("abc");
	const std::string after = response.toString();

	EXPECT_EQ(before,
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
	EXPECT_EQ(after,
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 3\r\n"
			  "\r\n"
			  "abc");
}

// Scenario: the body is replaced with a longer value.
//   Given : setBody("first") then setBody("second")
//   When  : toString() serializes it
//   Then  : only the last body is sent and Content-Length matches it
TEST(HttpResponseTest, SetBodyTwiceKeepsLastValue)
{
	HttpResponse response;
	response.setBody("first");
	response.setBody("second");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 6\r\n"
			  "\r\n"
			  "second");
}

// Scenario: a filled body is cleared afterwards.
//   Given : setBody("abc") then setBody("")
//   When  : toString() serializes it
//   Then  : Content-Length goes back to 0 and no body bytes are sent
TEST(HttpResponseTest, EmptyBodyAfterBodyResetsContentLength)
{
	HttpResponse response;
	response.setBody("abc");
	response.setBody("");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
}

// Scenario: the body contains an embedded NUL byte.
//   Given : a 3-byte body "a\0b"
//   When  : toString() serializes it
//   Then  : Content-Length is 3 and the NUL byte is not truncated
TEST(HttpResponseTest, BodyWithEmbeddedNulIsNotTruncated)
{
	const std::string body("a\0b", 3);
	HttpResponse response;
	response.setBody(body);

	std::string expected = "HTTP/1.1 200 OK\r\n"
						   "Content-Length: 3\r\n"
						   "\r\n";
	expected += body;
	EXPECT_EQ(response.toString(), expected);
}

// Scenario: the original response is changed after being copied.
//   Given : a copy of a response, then the original gets a new body
//   When  : the copy is serialized
//   Then  : the copy still carries the old body (copies are independent)
TEST(HttpResponseTest, CopyIsIndependentFromOriginal)
{
	HttpResponse original;
	original.setBody("one");

	HttpResponse copy(original);
	original.setBody("two");

	EXPECT_EQ(copy.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 3\r\n"
			  "\r\n"
			  "one");
	EXPECT_EQ(original.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 3\r\n"
			  "\r\n"
			  "two");
}

// Scenario: a response is assigned to itself through a reference.
//   Given : a configured response aliased by a reference
//   When  : operator= runs with both sides pointing to the same object
//   Then  : the state is untouched and serialization is unchanged
TEST(HttpResponseTest, SelfAssignmentPreservesSerialization)
{
	HttpResponse response;
	response.setStatus(201, "Created");
	response.setHeader("X-Id", "7");
	response.setBody("ok");

	HttpResponse& same = response;
	response = same;

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 201 Created\r\n"
			  "Content-Length: 2\r\n"
			  "X-Id: 7\r\n"
			  "\r\n"
			  "ok");
}

// Scenario: the status is set without a reason phrase.
//   Given : setStatus(200, "")
//   When  : toString() serializes it
//   Then  : the status line has no trailing space before CRLF
TEST(HttpResponseTest, EmptyReasonPhraseOmitsTrailingSpace)
{
	HttpResponse response;
	response.setStatus(200, "");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
}

// Scenario: the status is replaced before serialization.
//   Given : setStatus(404, "Not Found") then setStatus(500, ...)
//   When  : toString() serializes it
//   Then  : only the last code and reason are used
TEST(HttpResponseTest, SetStatusTwiceKeepsLastValue)
{
	HttpResponse response;
	response.setStatus(404, "Not Found");
	response.setStatus(500, "Internal Server Error");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 500 Internal Server Error\r\n"
			  "Content-Length: 0\r\n"
			  "\r\n");
}

// Scenario: serialization is requested through a const reference.
//   Given : a response accessed as const HttpResponse&
//   When  : toString() is called on it
//   Then  : it compiles and returns the expected bytes (const-correctness)
TEST(HttpResponseTest, ToStringWorksOnConstObject)
{
	HttpResponse response;
	response.setBody("hi");

	const HttpResponse& view = response;

	EXPECT_EQ(view.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 2\r\n"
			  "\r\n"
			  "hi");
}

// Scenario: the same header is set with two different cases.
//   Given : setHeader("Content-Type", ...) then setHeader("content-type", ...)
//   When  : toString() serializes it
//   Then  : a single line remains, with the name and value of the last call
TEST(HttpResponseTest, DuplicateHeaderNameWithDifferentCaseIsCollapsed)
{
	HttpResponse response;
	response.setHeader("Content-Type", "text/plain");
	response.setHeader("content-type", "text/html");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "content-type: text/html\r\n"
			  "\r\n");
}

// Scenario: a header tries to inject CRLF into the response.
//   Given : one header with CRLF in the value and one with CRLF in the name,
//           followed by a valid header
//   When  : toString() serializes it
//   Then  : the malicious headers are dropped and the valid one is kept
TEST(HttpResponseTest, HeaderWithCrlfIsRejected)
{
	HttpResponse response;
	response.setHeader("X-Bad", "x\r\nSet-Cookie: y=1");
	response.setHeader("X\r\nBad", "v");
	response.setHeader("X-Good", "ok");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 200 OK\r\n"
			  "Content-Length: 0\r\n"
			  "X-Good: ok\r\n"
			  "\r\n");
}

// Scenario: a 204 response is built with body bytes and Content-Length.
//   Given : setStatus(204, ...), a body and an explicit Content-Length
//   When  : toString() serializes it
//   Then  : neither Content-Length nor the body are sent
TEST(HttpResponseTest, NoContentStatusOmitsBodyAndContentLength)
{
	HttpResponse response;
	response.setStatus(204, "No Content");
	response.setHeader("Content-Length", "9");
	response.setBody("should not be sent");

	EXPECT_EQ(response.toString(), "HTTP/1.1 204 No Content\r\n\r\n");
}

// Scenario: a 304 response carries an explicit Content-Length.
//   Given : setStatus(304, ...), setHeader("Content-Length", "42") and a body
//   When  : toString() serializes it
//   Then  : the body is suppressed but the explicit Content-Length is kept
TEST(HttpResponseTest, NotModifiedKeepsExplicitContentLengthWithoutBody)
{
	HttpResponse response;
	response.setStatus(304, "Not Modified");
	response.setHeader("Content-Length", "42");
	response.setBody("x");

	EXPECT_EQ(response.toString(),
			  "HTTP/1.1 304 Not Modified\r\n"
			  "Content-Length: 42\r\n"
			  "\r\n");
}

// Scenario: an informational (1xx) response is built with body bytes.
//   Given : setStatus(100, ...), a body and an explicit Content-Length
//   When  : toString() serializes it
//   Then  : neither Content-Length nor the body are sent
TEST(HttpResponseTest, InformationalStatusOmitsBodyAndContentLength)
{
	HttpResponse response;
	response.setStatus(100, "Continue");
	response.setHeader("Content-Length", "1");
	response.setBody("x");

	EXPECT_EQ(response.toString(), "HTTP/1.1 100 Continue\r\n\r\n");
}
