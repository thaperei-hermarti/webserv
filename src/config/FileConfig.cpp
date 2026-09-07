/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thaperei <thaperei@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 18:27:26 by thaperei          #+#    #+#             */
/*   Updated: 2026/08/19 19:27:56 by thaperei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/FileConfig.hpp"

FileConfig::FileConfig()
{
}

FileConfig::FileConfig(std::string const path) : path_(path)
{
}

FileConfig::~FileConfig()
{
}

std::string FileConfig::getPath() const
{
	return path_;
}

std::string FileConfig::readFile()
{
	if (path_.empty() || path_.length() == 0)
		return std::string();
	std::ifstream file(path_.c_str());
	if (!file || !file.is_open())
		return std::string();

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int FileConfig::getTypePath(std::string const path)
{
	struct stat st;
	int result;

	result = stat(path.c_str(), &st);
	if (result == 0)
	{
		if (st.st_mode & S_IFREG)
			return 1;
		else if (st.st_mode & S_IFDIR)
			return 2;
		else
			return 3;
	}
	return -1;
}

int FileConfig::checkFileAccess(std::string const path, int mode)
{
	return access(path.c_str(), mode);
}

int FileConfig::isFileExistsAndReadable(std::string const path,
										std::string const index)
{
	if (getTypePath(path) == 1 && checkFileAccess(path, R_OK) == 0)
		return 1;
	else if (getTypePath(path + index) == 1 &&
			 checkFileAccess(path + index, R_OK) == 0)
		return 1;
	return 0;
}
