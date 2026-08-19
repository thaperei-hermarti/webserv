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

#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <string>

namespace
{
	class TempFile
	{
		public:
			TempFile(): _path()
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

		private:
			std::string _path;
	};
}

TEST(FileConfigTest, IsFileExistsForExistingFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	FileConfig config(file.getPath());
	EXPECT_TRUE(config.isFileExists());
}

TEST(FileConfigTest, IsFileExistsForMissingFile)
{
	FileConfig config("/tmp/webserv_file_config_missing_12345");
	EXPECT_FALSE(config.isFileExists());
}

TEST(FileConfigTest, IsFileExistsForDefaultConstructed)
{
	FileConfig config;
	EXPECT_FALSE(config.isFileExists());
}

TEST(FileConfigTest, IsReadableForReadableFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	FileConfig config(file.getPath());
	EXPECT_TRUE(config.isReadable());
}

TEST(FileConfigTest, IsReadableForMissingFile)
{
	FileConfig config("/tmp/webserv_file_config_missing_12345");
	EXPECT_FALSE(config.isReadable());
}

TEST(FileConfigTest, IsReadableForUnreadableFile)
{
	TempFile file;

	ASSERT_FALSE(file.getPath().empty());
	ASSERT_TRUE(file.makeUnreadable());
	FileConfig config(file.getPath());
	EXPECT_FALSE(config.isReadable());
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