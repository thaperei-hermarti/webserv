/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_config_test.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:12 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:24:12 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerConfig.hpp"

#include <gtest/gtest.h>

namespace
{
LocationConfig locationWithPath(const std::string& path)
{
	LocationConfig location;
	location.path_ = path;
	return location;
}
} // namespace

// Scenario: the server has no location blocks.
//   Given : an empty locations_ vector
//   When  : any URI is matched
//   Then  : no location is returned instead of dereferencing an empty vector
TEST(ServerConfigTest, EmptyLocationsHaveNoMatch)
{
	ServerConfig server;

	EXPECT_EQ(nullptr, server.matchLocation("/"));
}

// Scenario: the URI exactly equals a location path.
//   Given : one location "/api"
//   When  : "/api" is matched
//   Then  : the same LocationConfig is returned
TEST(ServerConfigTest, ExactPathMatches)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/api"));

	LocationConfig* matched = server.matchLocation("/api");

	ASSERT_NE(nullptr, matched);
	EXPECT_EQ(&server.locations_[0], matched);
	EXPECT_EQ("/api", matched->path_);
}

// Scenario: several prefixes could match the same URI.
//   Given : locations "/", "/images" and "/images/thumbnails"
//   When  : "/images/thumbnails/cat.png" is matched
//   Then  : the longest prefix wins
TEST(ServerConfigTest, LongestPrefixWins)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/"));
	server.locations_.push_back(locationWithPath("/images"));
	server.locations_.push_back(locationWithPath("/images/thumbnails"));

	LocationConfig* matched =
		server.matchLocation("/images/thumbnails/cat.png");

	ASSERT_NE(nullptr, matched);
	EXPECT_EQ("/images/thumbnails", matched->path_);
}

// Scenario: the URI only shares characters with the prefix, not a path boundary.
//   Given : one location "/images"
//   When  : "/imagesfoo" is matched
//   Then  : the location does not match
TEST(ServerConfigTest, PrefixRequiresPathBoundary)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/images"));

	EXPECT_EQ(nullptr, server.matchLocation("/imagesfoo"));
}

// Scenario: the root location is the catch-all location.
//   Given : one location "/"
//   When  : an arbitrary absolute path or root path with a query is matched
//   Then  : the root location is returned
TEST(ServerConfigTest, RootLocationMatchesAnyPath)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/"));

	ASSERT_NE(nullptr, server.matchLocation("/anything/here"));
	ASSERT_NE(nullptr, server.matchLocation("/?from=home"));
	EXPECT_EQ("/", server.matchLocation("/anything/here")->path_);
}

// Scenario: a location ending in a slash matches descendants.
//   Given : locations "/assets/" and "/assets/css/"
//   When  : "/assets/css/app.css" is matched
//   Then  : the longer trailing-slash prefix wins
TEST(ServerConfigTest, TrailingSlashPrefixMatchesDescendants)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/assets/"));
	server.locations_.push_back(locationWithPath("/assets/css/"));

	LocationConfig* matched = server.matchLocation("/assets/css/app.css");

	ASSERT_NE(nullptr, matched);
	EXPECT_EQ("/assets/css/", matched->path_);
}

// Scenario: a trailing-slash location does not match its parent path.
//   Given : one location "/assets/"
//   When  : "/assets" is matched
//   Then  : the location does not match
TEST(ServerConfigTest, TrailingSlashPrefixDoesNotMatchParent)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/assets/"));

	EXPECT_EQ(nullptr, server.matchLocation("/assets"));
}

// Scenario: the request URI carries a query string.
//   Given : one location "/search"
//   When  : "/search?q=webserv" is matched
//   Then  : the query string is ignored for location selection
TEST(ServerConfigTest, QueryStringIsIgnored)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/search"));

	LocationConfig* matched = server.matchLocation("/search?q=webserv");

	ASSERT_NE(nullptr, matched);
	EXPECT_EQ("/search", matched->path_);
}

// Scenario: no configured prefix matches the URI.
//   Given : one location "/api"
//   When  : "/static/app.js" is matched
//   Then  : no location is returned
TEST(ServerConfigTest, NoMatchingLocationReturnsNull)
{
	ServerConfig server;
	server.locations_.push_back(locationWithPath("/api"));

	EXPECT_EQ(nullptr, server.matchLocation("/static/app.js"));
}
