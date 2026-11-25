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
#include <string>
#include <vector>
#include <unistd.h>

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
