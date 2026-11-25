/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   static_file_handler_test.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:25:59 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:26:00 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/StaticFileHandler.hpp"

#include "config/LocationConfig.hpp"
#include "http/request/HttpRequest.hpp"

#include <gtest/gtest.h>

namespace
{
LocationConfig locationWithRoot(const std::string& path,
								const std::string& root)
{
	LocationConfig config;
	config.path_ = path;
	config.root_ = root;
	return config;
}
} // namespace

// Scenario: the subject's location root example.
//   Given : location "/kapouet" rooted at "/tmp/www"
//   When  : "/kapouet/pouic" is resolved
//   Then  : the location prefix is replaced by the root
TEST(StaticFileHandlerTest, ResolvePathUsesLocationRoot)
{
	LocationConfig config = locationWithRoot("/kapouet", "/tmp/www");

	EXPECT_EQ("/tmp/www/pouic",
			  StaticFileHandler::resolvePath("/kapouet/pouic", config));
}

// Scenario: a URI with several path segments below the location.
//   Given : location "/kapouet" rooted at "/tmp/www"
//   When  : "/kapouet/pouic/toto" is resolved
//   Then  : every segment after the location prefix is preserved
TEST(StaticFileHandlerTest, ResolvePathKeepsNestedPath)
{
	LocationConfig config = locationWithRoot("/kapouet", "/tmp/www");

	EXPECT_EQ("/tmp/www/pouic/toto",
			  StaticFileHandler::resolvePath("/kapouet/pouic/toto", config));
}

// Scenario: the request points at a directory with a trailing slash.
//   Given : location "/kapouet" rooted at "/tmp/www"
//   When  : "/kapouet/pouic/" is resolved
//   Then  : the trailing slash is preserved for directory handling
TEST(StaticFileHandlerTest, ResolvePathKeepsDirectoryTrailingSlash)
{
	LocationConfig config = locationWithRoot("/kapouet", "/tmp/www");

	EXPECT_EQ("/tmp/www/pouic/",
			  StaticFileHandler::resolvePath("/kapouet/pouic/", config));
}

// Scenario: the URI is exactly the location prefix.
//   Given : location "/kapouet" rooted at "/tmp/www"
//   When  : "/kapouet" is resolved
//   Then  : the location root itself is returned
TEST(StaticFileHandlerTest, ResolvePathUsesRootForExactLocation)
{
	LocationConfig config = locationWithRoot("/kapouet", "/tmp/www");

	EXPECT_EQ("/tmp/www", StaticFileHandler::resolvePath("/kapouet", config));
}

// Scenario: both the location and the root end with a slash.
//   Given : location "/assets/" rooted at "/srv/assets/"
//   When  : "/assets/css/app.css" is resolved
//   Then  : joining the paths does not produce a double slash
TEST(StaticFileHandlerTest, ResolvePathJoinsTrailingSlashes)
{
	LocationConfig config = locationWithRoot("/assets/", "/srv/assets/");

	EXPECT_EQ("/srv/assets/css/app.css",
			  StaticFileHandler::resolvePath("/assets/css/app.css", config));
}

// Scenario: the root location maps the URI directly below the root.
//   Given : location "/" rooted at "www/"
//   When  : "/index.html" is resolved
//   Then  : the leading location slash is not duplicated
TEST(StaticFileHandlerTest, ResolvePathMatchesRootLocation)
{
	LocationConfig config = locationWithRoot("/", "www/");

	EXPECT_EQ("www/index.html",
			  StaticFileHandler::resolvePath("/index.html", config));
}

// Scenario: the URI carries a query string.
//   Given : location "/search" rooted at "/tmp/www"
//   When  : "/search/results?q=webserv" is resolved
//   Then  : the query string is excluded from the filesystem path
TEST(StaticFileHandlerTest, ResolvePathIgnoresQueryString)
{
	LocationConfig config = locationWithRoot("/search", "/tmp/www");

	EXPECT_EQ(
		"/tmp/www/results",
		StaticFileHandler::resolvePath("/search/results?q=webserv", config));
}

// Scenario: the URI does not belong to the location.
//   Given : location "/api" rooted at "/tmp/www"
//   When  : "/static/app.js" is resolved
//   Then  : no filesystem path is returned
TEST(StaticFileHandlerTest, ResolvePathRejectsUriOutsideLocation)
{
	LocationConfig config = locationWithRoot("/api", "/tmp/www");

	EXPECT_EQ("", StaticFileHandler::resolvePath("/static/app.js", config));
}

// Scenario: the URI only shares a character prefix with the location.
//   Given : location "/api" rooted at "/tmp/www"
//   When  : "/apix" is resolved
//   Then  : the location boundary is not crossed
TEST(StaticFileHandlerTest, ResolvePathRejectsPartialSegmentMatch)
{
	LocationConfig config = locationWithRoot("/api", "/tmp/www");

	EXPECT_EQ("", StaticFileHandler::resolvePath("/apix", config));
}

// Scenario: the URI attempts to escape the location root.
//   Given : location "/" rooted at "/tmp/www"
//   When  : "/../etc/passwd" is resolved
//   Then  : the parent segment is rejected
TEST(StaticFileHandlerTest, ResolvePathRejectsParentTraversal)
{
	LocationConfig config = locationWithRoot("/", "/tmp/www");

	EXPECT_EQ("", StaticFileHandler::resolvePath("/../etc/passwd", config));
}

// Scenario: the resolved file does not exist.
//   Given : a missing root and URI
//   When  : the handler processes the request
//   Then  : it returns 404 without trying to serve a file
TEST(StaticFileHandlerTest, HandleReturnsNotFoundForMissingFile)
{
	StaticFileHandler handler;
	LocationConfig config =
		locationWithRoot("/", "/tmp/webserv_static_handler_missing_12345");
	HttpRequest request;
	request.setUri("/missing.txt");

	EXPECT_EQ(404, handler.handle(request, config).getStatusCode());
}

// Scenario: the request URI contains a traversal segment.
//   Given : location "/" rooted at "/tmp/www"
//   When  : "/../etc/passwd" is handled
//   Then  : the handler rejects the request as bad input
TEST(StaticFileHandlerTest, HandleReturnsBadRequestForTraversal)
{
	StaticFileHandler handler;
	LocationConfig config = locationWithRoot("/", "/tmp/www");
	HttpRequest request;
	request.setUri("/../etc/passwd");

	EXPECT_EQ(400, handler.handle(request, config).getStatusCode());
}
