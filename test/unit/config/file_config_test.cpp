/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   file_config_test.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thaperei <thaperei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 18:39:14 by thaperei          #+#    #+#             */
/*   Updated: 2026/08/19 19:57:59 by thaperei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/FileConfig.hpp"

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>
#include <fstream>
#include <string>

namespace
{
class TempFile
{
  private:
	std::string _path;

  public:
	TempFile() : _path()
	{
		char tmpl[] = "/tmp/webserv_file_config_XXXXXX";
		int fd = mkstemp(tmpl);
		if (fd != -1)
		{
			_path = tmpl;
			close(fd);
		}
	}

	~TempFile()
	{
		if (!_path.empty())
		{
			std::remove(_path.c_str());
		}
	}

	bool writeContent(std::string const& content) const
	{
		std::ofstream out(_path.c_str(), std::ios::out | std::ios::trunc);
		if (!out.is_open())
		{
			return false;
		}
		out << content;
		return true;
	}

	bool makeUnreadable() const
	{
		return chmod(_path.c_str(), 0) == 0;
	}

	std::string const& getPath() const
	{
		return _path;
	}
};
} // namespace

TEST(FileConfigTest, GetPathReturnsConfiguredPath)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	FileConfig config(file.getPath());
	EXPECT_EQ(file.getPath(), config.getPath());
}

TEST(FileConfigTest, DefaultPathIsEmpty)
{
	FileConfig config;
	EXPECT_EQ("", config.getPath());
}

TEST(FileConfigTest, IsFileExistsAndReadableForExistingFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	EXPECT_TRUE(FileConfig::isFileExistsAndReadable(file.getPath(), ""));
}

TEST(FileConfigTest, IsFileExistsAndReadableForMissingFile)
{
	EXPECT_FALSE(FileConfig::isFileExistsAndReadable(
		"/tmp/webserv_file_config_missing_12345", ""));
}

TEST(FileConfigTest, IsFileExistsAndReadableUsesIndex)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	std::string const filename =
		file.getPath().substr(file.getPath().find_last_of('/') + 1);
	EXPECT_TRUE(FileConfig::isFileExistsAndReadable("/tmp/", filename));
}

TEST(FileConfigTest, ReadFileReturnsContent)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	ASSERT_TRUE(file.writeContent("server {\n\tlisten 8080;\n}\n"));
	FileConfig config(file.getPath());
	EXPECT_EQ("server {\n\tlisten 8080;\n}\n", config.readFile());
}

TEST(FileConfigTest, ReadFileReturnsEmptyForEmptyFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	ASSERT_TRUE(file.writeContent(""));
	FileConfig config(file.getPath());
	EXPECT_EQ("", config.readFile());
}

TEST(FileConfigTest, ReadFileReturnsEmptyForMissingFile)
{
	FileConfig config("/tmp/webserv_file_config_missing_12345");
	EXPECT_EQ("", config.readFile());
}

TEST(FileConfigTest, ReadFileReturnsEmptyForUnreadableFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	ASSERT_TRUE(file.writeContent("content"));
	ASSERT_TRUE(file.makeUnreadable());
	FileConfig config(file.getPath());
	EXPECT_EQ("", config.readFile());
}
