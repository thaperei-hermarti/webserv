/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thaperei <thaperei@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 15:26:35 by thaperei          #+#    #+#             */
/*   Updated: 2026/08/19 19:28:02 by thaperei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CONFIG_FILECONFIG_HPP
#define WEBSERV_CONFIG_FILECONFIG_HPP

#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

class FileConfig
{
	std::string _path;

  public:
	FileConfig();
	FileConfig(std::string const path);
	~FileConfig();

	std::string getPath() const;
	std::string readFile();
	bool isReadable() const;
	bool isFileExists() const;
};

#endif
