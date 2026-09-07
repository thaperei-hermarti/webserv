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
#include "lexer/Lexer.hpp"

class ConfigParser
{
  public:
	ConfigParser();
	ConfigParser(Lexer& lexer);
	~ConfigParser();
	ConfigParser(const ConfigParser& other);
	ConfigParser& operator=(const ConfigParser& other);

	std::vector<ServerConfig> parseConfig();
	Lexer getLexer() const;

  private:
	Lexer lexer_;

	const t_lexer_token& peek() const;
	const t_lexer_token& consume(t_token type, const std::string& expected);
	const t_lexer_token& consumeValue(const std::string& expected);
	std::string parsePathValue(const std::string& expected);
	int parseNumber(const std::string& expected);
	void rejectDuplicate(bool& seen, const std::string& directive);

	ServerConfig parseServerBlock();
	LocationConfig parseLocationBlock();
	void parseListenDirective(ServerConfig& server);
	void parseServerNameDirective(ServerConfig& server);
	void parseRootDirective(ServerConfig* server, LocationConfig* location);
	void parseIndexDirective(ServerConfig* server, LocationConfig* location);
	void parseClientMaxBodySizeDirective(ServerConfig* server,
										 LocationConfig* location);
	void parseErrorPageDirective(ServerConfig* server,
								 LocationConfig* location);
	void parseMethodDirective(LocationConfig& location);
	void parseAutoindexDirective(LocationConfig& location);
	void parseUploadDirDirective(LocationConfig& location);
	void parseReturnDirective(LocationConfig& location);
	void parseCgiPassDirective(LocationConfig& location);
};

#endif
