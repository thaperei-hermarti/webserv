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

FileConfig::FileConfig() { }

FileConfig::FileConfig(std::string const path): _path(path) { }

FileConfig::~FileConfig() { }

std::string FileConfig::getPath() const { return _path; }

std::string FileConfig::readFile()
{
	std::ifstream file(_path.c_str());
	std::stringstream buffer;

	if (!file.is_open())
		return std::string();
	buffer << file.rdbuf();
	return buffer.str();
}

bool FileConfig::isReadable() const { return access(_path.c_str(), R_OK) == 0; }

bool FileConfig::isFileExists() const
{
	struct stat st;

	return stat(_path.c_str(), &st) == 0;
}
