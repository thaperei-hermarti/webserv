/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hermarti <hermarti@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 13:21:37 by hermarti          #+#    #+#             */
/*   Updated: 2026/08/14 13:21:38 by hermarti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
