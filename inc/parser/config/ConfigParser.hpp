/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:16:27 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:16:30 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_PARSER_CONFIG_CONFIGPARSER_HPP
#define WEBSERV_PARSER_CONFIG_CONFIGPARSER_HPP

#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"

class ConfigParser
{
  public:
	ConfigParser();
	~ConfigParser();
	ConfigParser(const ConfigParser& other);
	ConfigParser& operator=(const ConfigParser& other);

	std::vector<ServerConfig> parseFile(const std::string& path);

  private:
	ServerConfig parseServerBlock();
	LocationConfig parseLocationBlock();
};

#endif
