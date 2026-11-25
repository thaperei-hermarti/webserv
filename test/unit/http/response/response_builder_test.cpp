/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response_builder_test.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:24:33 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:24:34 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/response/ResponseBuilder.hpp"

#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

#include "config/ServerConfig.hpp"

namespace
{
class TempDir
{
  private:
	std::string path_;
	std::vector<std::string> files_;

  public:
	TempDir() : path_(), files_()
	{
		char tmpl[] = "/tmp/webserv_response_builder_XXXXXX";
		char* created = mkdtemp(tmpl);
		if (created != NULL)
		{
			path_ = created;
		}
	}

	~TempDir()
	{
		if (!path_.empty())
		{
			for (std::size_t i = 0; i < files_.size(); ++i)
			{
				std::remove((path_ + "/" + files_[i]).c_str());
			}
			rmdir(path_.c_str());
		}
	}

	bool ok() const
	{
		return !path_.empty();
	}

	std::string path(const std::string& name) const
	{
		return path_ + "/" + name;
	}

	bool write(const std::string& name, const std::string& content)
	{
		std::ofstream out(path(name).c_str(),
						  std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open())
		{
			return false;
		}
		out.write(content.data(), static_cast<std::streamsize>(content.size()));
		if (!out.good())
		{
			return false;
		}
		files_.push_back(name);
		return true;
	}
};
} // namespace

// Scenario: a file exists at the resolved path.
//   Given : a temporary file containing "hello\n"
//   When  : buildFromFile() builds the response
//   Then  : the response body is the file contents
TEST(ResponseBuilderTest, BuildFromFileReturnsFileBody)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("page.html", "hello\n"));

	HttpResponse response =
		ResponseBuilder::buildFromFile(dir.path("page.html"));
	const std::string serialized = response.toString();
	const std::size_t body_start = serialized.find("\r\n\r\n");

	ASSERT_NE(std::string::npos, body_start);
	EXPECT_EQ(200, response.getStatusCode());
	EXPECT_EQ("hello\n", serialized.substr(body_start + 4));
}

// Scenario: the file body has a known size.
//   Given : a temporary file containing "hello world\n"
//   When  : buildFromFile() builds the response
//   Then  : Content-Length matches the file size
TEST(ResponseBuilderTest, BuildFromFileSetsContentLength)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("page.html", "hello world\n"));

	HttpResponse response =
		ResponseBuilder::buildFromFile(dir.path("page.html"));

	EXPECT_NE(std::string::npos,
			  response.toString().find("\r\nContent-Length: 12\r\n"));
}

// Scenario: the builder maps common extensions to content types.
//   Given : files with html, css, txt, png and unknown extensions
//   When  : buildFromFile() builds each response
//   Then  : each response carries the matching Content-Type
TEST(ResponseBuilderTest, BuildFromFileSetsContentTypeByExtension)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("page.html", "html"));
	ASSERT_TRUE(dir.write("style.css", "css"));
	ASSERT_TRUE(dir.write("notes.txt", "txt"));
	ASSERT_TRUE(dir.write("image.png", "png"));
	ASSERT_TRUE(dir.write("data.bin", "bin"));

	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("page.html"))
				  .toString()
				  .find("\r\nContent-Type: text/html\r\n"));
	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("style.css"))
				  .toString()
				  .find("\r\nContent-Type: text/css\r\n"));
	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("notes.txt"))
				  .toString()
				  .find("\r\nContent-Type: text/plain\r\n"));
	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("image.png"))
				  .toString()
				  .find("\r\nContent-Type: image/png\r\n"));
	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("data.bin"))
				  .toString()
				  .find("\r\nContent-Type: application/octet-stream\r\n"));
}

// Scenario: the response is generated at request time.
//   Given : a valid file response
//   When  : toString() serializes it
//   Then  : Date uses the IMF-fixdate format with a GMT suffix
TEST(ResponseBuilderTest, BuildFromFileSetsDateHeader)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("page.html", "hello"));

	const std::string serialized =
		ResponseBuilder::buildFromFile(dir.path("page.html")).toString();
	const std::string prefix = "\r\nDate: ";
	const std::size_t start = serialized.find(prefix);

	ASSERT_NE(std::string::npos, start);
	const std::size_t end = serialized.find("\r\n", start + prefix.size());
	ASSERT_NE(std::string::npos, end);
	const std::string date =
		serialized.substr(start + prefix.size(), end - start - prefix.size());
	EXPECT_EQ(29u, date.size());
	EXPECT_EQ(" GMT", date.substr(date.size() - 4));
}

// Scenario: the server identifies itself in every generated response.
//   Given : a valid file response
//   When  : toString() serializes it
//   Then  : the Server header identifies webserv
TEST(ResponseBuilderTest, BuildFromFileSetsServerHeader)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("page.html", "hello"));

	EXPECT_NE(std::string::npos,
			  ResponseBuilder::buildFromFile(dir.path("page.html"))
				  .toString()
				  .find("\r\nServer: webserv\r\n"));
}

// Scenario: no custom error page is configured.
//   Given : a ServerConfig without a 404 error page
//   When  : buildErrorResponse() builds the response
//   Then  : the default cat page and standard headers are returned
TEST(ResponseBuilderTest, BuildErrorResponseUsesDefaultCatPage)
{
	ServerConfig config;

	HttpResponse response = ResponseBuilder::buildErrorResponse(404, config);
	const std::string serialized = response.toString();

	EXPECT_EQ(404, response.getStatusCode());
	EXPECT_NE(std::string::npos, serialized.find("HTTP/1.1 404 Not Found\r\n"));
	EXPECT_NE(std::string::npos,
			  serialized.find("\r\nContent-Type: text/html\r\n"));
	EXPECT_NE(std::string::npos, serialized.find("<h1>404 Not Found</h1>"));
	EXPECT_NE(std::string::npos, serialized.find("/error_pages/404.jpg"));
	EXPECT_EQ(std::string::npos, serialized.find("http.cat"));
	EXPECT_NE(std::string::npos, serialized.find("\r\nDate: "));
	EXPECT_NE(std::string::npos, serialized.find("\r\nServer: webserv\r\n"));
}

// Scenario: the fallback is a static HTML page.
//   Given : a request with no configured error page
//   When  : buildErrorResponse() builds the default page
//   Then  : the response body is a complete styled website
TEST(ResponseBuilderTest, BuildErrorResponseReturnsStandaloneHtmlPage)
{
	ServerConfig config;

	const std::string serialized =
		ResponseBuilder::buildErrorResponse(404, config).toString();

	EXPECT_NE(std::string::npos, serialized.find("<!DOCTYPE html>"));
	EXPECT_NE(std::string::npos, serialized.find("<meta name=\"viewport\""));
	EXPECT_NE(std::string::npos, serialized.find("<style>"));
	EXPECT_NE(std::string::npos, serialized.find("<h1>404 Not Found</h1>"));
	EXPECT_NE(std::string::npos, serialized.find("Back to home"));
	EXPECT_NE(std::string::npos, serialized.find("</html>"));
}

// Scenario: the fallback page is stored as a static file.
//   Given : www/error_pages/404.html
//   When  : buildErrorResponse() builds a 404 response
//   Then  : the response body is exactly that static file
TEST(ResponseBuilderTest, BuildErrorResponseLoadsStaticErrorPage)
{
	std::ifstream page("www/error_pages/404.html",
					   std::ios::in | std::ios::binary);
	ASSERT_TRUE(page.good());
	std::ostringstream expected;
	expected << page.rdbuf();

	ServerConfig config;
	const std::string serialized =
		ResponseBuilder::buildErrorResponse(404, config).toString();
	const std::size_t body_start = serialized.find("\r\n\r\n");

	ASSERT_NE(std::string::npos, body_start);
	EXPECT_EQ(expected.str(), serialized.substr(body_start + 4));
}

// Scenario: the default error pages ship with the server.
//   Given : the static HTML files used by the fallback pages
//   When  : the repository is inspected
//   Then  : every required page is readable
TEST(ResponseBuilderTest, DefaultErrorPagesShipWithServer)
{
	const char* PATHS[] = {"www/error_pages/400.html",
						   "www/error_pages/403.html",
						   "www/error_pages/404.html",
						   "www/error_pages/405.html",
						   "www/error_pages/413.html",
						   "www/error_pages/500.html"};

	for (std::size_t i = 0; i < sizeof(PATHS) / sizeof(PATHS[0]); ++i)
	{
		std::ifstream page(PATHS[i], std::ios::in | std::ios::binary);
		EXPECT_TRUE(page.good()) << PATHS[i];
	}
}

// Scenario: the default error images ship with the server.
//   Given : the assets used by the fallback pages
//   When  : the repository is inspected
//   Then  : every required JPEG is readable
TEST(ResponseBuilderTest, DefaultErrorImagesShipWithServer)
{
	const char* PATHS[] = {"www/error_pages/400.jpg",
						   "www/error_pages/403.jpg",
						   "www/error_pages/404.jpg",
						   "www/error_pages/405.jpg",
						   "www/error_pages/413.jpg",
						   "www/error_pages/500.jpg"};

	for (std::size_t i = 0; i < sizeof(PATHS) / sizeof(PATHS[0]); ++i)
	{
		std::ifstream image(PATHS[i], std::ios::in | std::ios::binary);
		ASSERT_TRUE(image.good()) << PATHS[i];
		unsigned char magic[2] = {0, 0};
		image.read(reinterpret_cast<char*>(magic), 2);
		EXPECT_EQ(0xFF, static_cast<int>(magic[0])) << PATHS[i];
		EXPECT_EQ(0xD8, static_cast<int>(magic[1])) << PATHS[i];
	}
}

// Scenario: the server must produce every required default error page.
//   Given : codes 400, 403, 404, 405, 413 and 500
//   When  : buildErrorResponse() builds each response
//   Then  : each has its reason phrase and matching cat image
TEST(ResponseBuilderTest, BuildErrorResponseCoversRequiredCodes)
{
	const struct ErrorCase
	{
		int code;
		const char* reason;
	} CASES[] = {{400, "Bad Request"},
				 {403, "Forbidden"},
				 {404, "Not Found"},
				 {405, "Method Not Allowed"},
				 {413, "Payload Too Large"},
				 {500, "Internal Server Error"}};

	for (std::size_t i = 0; i < sizeof(CASES) / sizeof(CASES[0]); ++i)
	{
		std::ostringstream code;
		code << CASES[i].code;
		const ServerConfig CONFIG;
		HttpResponse response =
			ResponseBuilder::buildErrorResponse(CASES[i].code, CONFIG);
		const std::string serialized = response.toString();

		EXPECT_EQ(CASES[i].code, response.getStatusCode());
		EXPECT_NE(std::string::npos,
				  serialized.find("HTTP/1.1 " + code.str() + " " +
								  CASES[i].reason + "\r\n"));
		EXPECT_NE(std::string::npos,
				  serialized.find("/error_pages/" + code.str() + ".jpg"));
		EXPECT_NE(
			std::string::npos,
			serialized.find("data-static-error-page=\"" + code.str() + "\""));
		EXPECT_EQ(std::string::npos, serialized.find("http.cat"));
	}
}

// Scenario: a custom error page is configured and readable.
//   Given : a 404 mapping to an existing HTML file
//   When  : buildErrorResponse() builds the response
//   Then  : the custom body is served with the error status
TEST(ResponseBuilderTest, BuildErrorResponseUsesConfiguredPage)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());
	ASSERT_TRUE(dir.write("404.html", "custom 404\n"));

	ServerConfig config;
	config.error_pages_[404] = dir.path("404.html");

	HttpResponse response = ResponseBuilder::buildErrorResponse(404, config);
	const std::string serialized = response.toString();

	EXPECT_EQ(404, response.getStatusCode());
	EXPECT_NE(std::string::npos, serialized.find("HTTP/1.1 404 Not Found\r\n"));
	EXPECT_NE(std::string::npos,
			  serialized.find("\r\nContent-Type: text/html\r\n"));
	EXPECT_NE(std::string::npos, serialized.find("\r\n\r\ncustom 404\n"));
}

// Scenario: a configured error page cannot be read.
//   Given : a 404 mapping to a missing file
//   When  : buildErrorResponse() builds the response
//   Then  : the default cat page is used instead
TEST(ResponseBuilderTest, BuildErrorResponseFallsBackForMissingConfiguredPage)
{
	TempDir dir;
	ASSERT_TRUE(dir.ok());

	ServerConfig config;
	config.error_pages_[404] = dir.path("missing.html");

	HttpResponse response = ResponseBuilder::buildErrorResponse(404, config);
	const std::string serialized = response.toString();

	EXPECT_EQ(404, response.getStatusCode());
	EXPECT_NE(std::string::npos, serialized.find("<h1>404 Not Found</h1>"));
	EXPECT_NE(std::string::npos, serialized.find("/error_pages/404.jpg"));
}
