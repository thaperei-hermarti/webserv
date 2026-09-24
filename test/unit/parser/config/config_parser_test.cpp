/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config_parser_test.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:41 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:24:42 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser/config/ConfigParser.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace
{
std::vector<ServerConfig> parseContents(const std::string& contents)
{
	Lexer lexer(contents);
	ConfigParser parser(lexer);
	return parser.parseConfig();
}

void expectParseError(const std::string& contents)
{
	EXPECT_THROW(parseContents(contents), std::runtime_error);
}
} // namespace

TEST(ConfigParserTest, HeaderCompiles)
{
	SUCCEED();
}

TEST(ConfigParserTest, ParsesAllSupportedDirectives)
{
	std::vector<ServerConfig> servers =
		parseContents("server {\n"
					  " listen 127.0.0.1:8080;\n"
					  " server_name example.com www.example.com;\n"
					  " root /srv/www;\n"
					  " index home.html index.htm;\n"
					  " client_max_body_size 10m;\n"
					  " error_page 404 500 /errors/error.html;\n"
					  " location /files {\n"
					  "  method GET POST DELETE;\n"
					  "  autoindex on;\n"
					  "  index files.html;\n"
					  "  client_max_body_size 2m;\n"
					  "  upload_dir /srv/uploads;\n"
					  "  return 301 https://example.com/files;\n"
					  "  error_page 403 /errors/forbidden.html;\n"
					  "  cgi_pass .py /usr/bin/python3;\n"
					  " }\n"
					  "}\n");
	ASSERT_EQ(1u, servers.size());
	const ServerConfig& server = servers[0];
	EXPECT_EQ("127.0.0.1", server.host_);
	ASSERT_EQ(1u, server.ports_.size());
	EXPECT_EQ(8080, server.ports_[0]);
	ASSERT_EQ(2u, server.server_names_.size());
	EXPECT_EQ("/srv/www", server.root_);
	ASSERT_EQ(2u, server.index_file_.size());
	EXPECT_EQ("home.html", server.index_file_[0]);
	EXPECT_EQ("index.htm", server.index_file_[1]);
	EXPECT_EQ(10u * 1024u * 1024u, server.client_max_body_size_);
	EXPECT_EQ("/errors/error.html", server.error_pages_.at(404));
	EXPECT_EQ("/errors/error.html", server.error_pages_.at(500));

	ASSERT_EQ(1u, server.locations_.size());
	const LocationConfig& location = server.locations_[0];
	EXPECT_EQ("/files", location.path_);
	EXPECT_EQ("/srv/www", location.root_);
	ASSERT_EQ(1u, location.index_file_.size());
	EXPECT_EQ("files.html", location.index_file_[0]);
	EXPECT_EQ(2u * 1024u * 1024u, location.client_max_body_size_);
	ASSERT_EQ(3u, location.allowed_methods_.size());
	EXPECT_TRUE(location.autoindex_);
	EXPECT_TRUE(location.upload_enabled_);
	EXPECT_EQ("/srv/uploads", location.upload_store_);
	EXPECT_EQ("301 https://example.com/files", location.redirect_);
	EXPECT_EQ("/errors/forbidden.html", location.error_pages_.at(403));
	EXPECT_EQ("/usr/bin/python3", location.cgi_extensions_.at(".py"));
}

TEST(ConfigParserTest, AppliesDocumentedDefaults)
{
	std::vector<ServerConfig> servers =
		parseContents("server { root /srv/www; location / { } }");

	ASSERT_EQ(1u, servers.size());
	EXPECT_EQ("localhost", servers[0].host_);
	ASSERT_EQ(1u, servers[0].ports_.size());
	EXPECT_EQ(80, servers[0].ports_[0]);
	ASSERT_EQ(1u, servers[0].locations_.size());
	EXPECT_EQ("/srv/www", servers[0].locations_[0].root_);
	ASSERT_EQ(1u, servers[0].locations_[0].index_file_.size());
	EXPECT_EQ("index.html", servers[0].locations_[0].index_file_[0]);
	ASSERT_EQ(3u, servers[0].locations_[0].allowed_methods_.size());
}

TEST(ConfigParserTest, ParsesMultipleServerBlocks)
{
	std::vector<ServerConfig> servers =
		parseContents("server { listen 8080; root /one; }\n"
					  "server { listen 8081; root /two; }\n");

	ASSERT_EQ(2u, servers.size());
	EXPECT_EQ("/one", servers[0].root_);
	EXPECT_EQ(8080, servers[0].ports_[0]);
	EXPECT_EQ("/two", servers[1].root_);
	EXPECT_EQ(8081, servers[1].ports_[0]);
}

TEST(ConfigParserTest, RejectsMissingServerRoot)
{
	expectParseError("server { listen 8080; }");
}

TEST(ConfigParserTest, RejectsDuplicateSingleUseDirectives)
{
	expectParseError("server { root /one; root /two; }");
	expectParseError(
		"server { root /one; location / { method GET; method POST; } }");
	expectParseError("server { root /one; location / { cgi_pass .py /bin/a; "
					 "cgi_pass .py /bin/b; } }");
}

TEST(ConfigParserTest, RejectsInvalidValues)
{
	expectParseError("server { root /one; listen 0; }");
	expectParseError("server { root /one; listen 65536; }");
	expectParseError("server { root /one; client_max_body_size 1g; }");
	expectParseError("server { root /one; error_page 99 /error; }");
	expectParseError("server { root /one; location / { autoindex maybe; } }");
}

TEST(ConfigParserTest, RejectsMalformedStructureAndTraversal)
{
	expectParseError(
		"server { root /one; location / { location /nested { } } }");
	expectParseError("server { root /one; location /../secret { } }");
	expectParseError("server { root /one location / { } }");
	expectParseError("server { root /one; location / { autoindex on } }");
}
