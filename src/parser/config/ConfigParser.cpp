/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                  :+:      :+:    :+:   */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*   Created: 2026/08/12 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/12 17:38:43 by hermarti         ###   ########.fr       */
/*   Created: 2026/07/31 17:34:55 by hermarti          #+#    #+#             */
/*   Updated: 2026/07/31 17:38:43 by hermarti         ###   ########.fr       */
/*                                                                            */

#include "parser/config/ConfigParser.hpp"

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
	(void) other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	(void) other;
	return *this;
}

std::vector<ServerConfig> ConfigParser::parseFile(const std::string& path)
{
	(void) path;
	return std::vector<ServerConfig>();
}

ServerConfig ConfigParser::parseServerBlock()
{
	return ServerConfig();
}

LocationConfig ConfigParser::parseLocationBlock()
{
	return LocationConfig();
}
