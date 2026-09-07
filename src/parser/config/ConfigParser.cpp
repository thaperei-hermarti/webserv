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

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace
{
std::string describe(const t_lexer_token& token)
{
	std::ostringstream stream;
	stream << tokenTypeToString(token.type) << " '" << token.value << "' at "
		   << token.line << ":" << token.column;
	return stream.str();
}

bool word(const t_lexer_token& token, const std::string& value)
{
	return token.type == WORD && token.value == value;
}

int integer(const std::string& value)
{
	std::istringstream stream(value);
	int result;
	char extra;
	if (!(stream >> result) || stream >> extra)
		throw std::runtime_error("Invalid number '" + value + "'");
	return result;
}

bool hostname(const std::string& value)
{
	if (value.empty() || value[0] == '.' || value[value.size() - 1] == '.' ||
		value.find('*') != std::string::npos ||
		value.find('~') != std::string::npos)
		return false;
	for (std::size_t i = 0; i < value.size(); ++i)
		if (!std::isalnum(static_cast<unsigned char>(value[i])) &&
			value[i] != '-' && value[i] != '.')
			return false;
	return true;
}

bool path(const std::string& value)
{
	return !value.empty() && value.find("..") == std::string::npos &&
		   value.find('*') == std::string::npos &&
		   value.find('~') == std::string::npos &&
		   (value[0] == '/' ||
			std::isalpha(static_cast<unsigned char>(value[0])) != 0);
}
} // namespace

ConfigParser::ConfigParser() : lexer_()
{
}

ConfigParser::ConfigParser(Lexer& lexer) : lexer_(lexer)
{
	lexer_.tokenize();
}

ConfigParser::~ConfigParser()
{
}

ConfigParser::ConfigParser(const ConfigParser& other) : lexer_(other.lexer_)
{
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	if (this != &other)
		lexer_ = other.lexer_;
	return *this;
}

Lexer ConfigParser::getLexer() const
{
	return (lexer_);
}

const t_lexer_token& ConfigParser::peek() const
{
	return lexer_.peek();
}

const t_lexer_token& ConfigParser::consume(t_token type,
										   const std::string& expected)
{
	if (lexer_.peek().type != type)
		throw std::runtime_error("Expected '" + expected + "', got " +
								 describe(lexer_.peek()));
	return lexer_.advance();
}

const t_lexer_token& ConfigParser::consumeValue(const std::string& expected)
{
	t_token value = lexer_.peek().type;

	if (!(value == WORD || value == NUMBER || value == PATH || value == URL))
		throw std::runtime_error("Expected '" + expected + "', got " +
								 describe(lexer_.peek()));
	return lexer_.advance();
}

std::string ConfigParser::parsePathValue(const std::string& expected)
{
	t_lexer_token first = consumeValue(expected);
	if (first.type == WORD && first.value.find(".") == std::string::npos)
		first.type = PATH;
	std::string result = first.value;
	if (lexer_.peek().type == PATH &&
		lexer_.peek().column ==
			first.column + static_cast<int>(first.value.size()))
		result += lexer_.advance().value;
	if (!path(result))
		throw std::runtime_error("Invalid path '" + result + "'");
	return result;
}

int ConfigParser::parseNumber(const std::string& expected)
{
	return integer(consume(NUMBER, expected).value);
}

void ConfigParser::rejectDuplicate(bool& seen, const std::string& directive)
{
	if (seen)
		throw std::runtime_error("Duplicate directive '" + directive + "'");
	seen = true;
}

std::vector<ServerConfig> ConfigParser::parseConfig()
{
	std::vector<ServerConfig> result;
	while (lexer_.peek().type != EOF_TOKEN)
		result.push_back(parseServerBlock());
	return result;
}

ServerConfig ConfigParser::parseServerBlock()
{
	if (!word(lexer_.peek(), "server"))
		throw std::runtime_error("Expected 'server', got " +
								 describe(lexer_.peek()));
	consume(WORD, "server");
	consume(BRACE, "{");
	ServerConfig server;
	bool rootSeen = false, listenSeen = false, nameSeen = false,
		 indexSeen = false, bodySizeSeen = false;

	while (lexer_.peek().type != CBRACE)
	{
		if (lexer_.peek().type == EOF_TOKEN)
			throw std::runtime_error("Unclosed server block");
		if (word(lexer_.peek(), "location"))
			server.locations_.push_back(parseLocationBlock());
		else if (word(lexer_.peek(), "root"))
		{
			rejectDuplicate(rootSeen, "root");
			parseRootDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "listen"))
		{
			rejectDuplicate(listenSeen, "listen");
			parseListenDirective(server);
		}
		else if (word(lexer_.peek(), "server_name"))
		{
			rejectDuplicate(nameSeen, "server_name");
			parseServerNameDirective(server);
		}
		else if (word(lexer_.peek(), "index"))
		{
			rejectDuplicate(indexSeen, "index");
			parseIndexDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "client_max_body_size"))
		{
			rejectDuplicate(bodySizeSeen, "client_max_body_size");
			parseClientMaxBodySizeDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "error_page"))
			parseErrorPageDirective(&server, NULL);
		else
			throw std::runtime_error("Unknown server directive " +
									 describe(lexer_.peek()));
	}
	consume(CBRACE, "}");
	if (!rootSeen)
		throw std::runtime_error("Missing mandatory 'root' directive");
	for (std::size_t i = 0; i < server.locations_.size(); ++i)
		if (server.locations_[i].root_.empty())
			server.locations_[i].root_ = server.root_;
	return server;
}

LocationConfig ConfigParser::parseLocationBlock()
{
	consume(WORD, "location");
	LocationConfig location;
	location.path_ = parsePathValue("location path");
	consume(BRACE, "{");
	bool rootSeen = false, indexSeen = false, bodySizeSeen = false;
	bool methodSeen = false, autoindexSeen = false, uploadSeen = false;
	bool returnSeen = false;
	while (lexer_.peek().type != CBRACE)
	{
		if (lexer_.peek().type == EOF_TOKEN)
			throw std::runtime_error("Unclosed location block");
		if (word(peek(), "location"))
			throw std::runtime_error("Nested location blocks are not allowed");
		else if (word(peek(), "method"))
		{
			rejectDuplicate(methodSeen, "method");
			parseMethodDirective(location);
		}
		else if (word(peek(), "autoindex"))
		{
			rejectDuplicate(autoindexSeen, "autoindex");
			parseAutoindexDirective(location);
		}
		else if (word(peek(), "root"))
		{
			rejectDuplicate(rootSeen, "root");
			parseRootDirective(NULL, &location);
		}
		else if (word(peek(), "index"))
		{
			rejectDuplicate(indexSeen, "index");
			parseIndexDirective(NULL, &location);
		}
		else if (word(peek(), "client_max_body_size"))
		{
			rejectDuplicate(bodySizeSeen, "client_max_body_size");
			parseClientMaxBodySizeDirective(NULL, &location);
		}
		else if (word(peek(), "upload_dir"))
		{
			rejectDuplicate(uploadSeen, "upload_dir");
			parseUploadDirDirective(location);
		}
		else if (word(peek(), "return"))
		{
			rejectDuplicate(returnSeen, "return");
			parseReturnDirective(location);
		}
		else if (word(peek(), "error_page"))
			parseErrorPageDirective(NULL, &location);
		else if (word(peek(), "cgi_pass"))
			parseCgiPassDirective(location);
		else
			throw std::runtime_error("Unknown location directive " +
									 describe(peek()));
	}
	consume(CBRACE, "}");
	return location;
}

void ConfigParser::parseListenDirective(ServerConfig& server)
{
	consume(WORD, "listen");
	if (peek().type == NUMBER)
	{
		const int firstPart = parseNumber("port or host");
		if (peek().type == WORD && peek().value.size() > 1 &&
			peek().value[0] == '.')
		{
			const std::string host =
				toString(firstPart) + lexer_.advance().value;
			if (!hostname(host))
				throw std::runtime_error("Invalid host '" + host + "'");
			consume(COLON, ":");
			server.host_ = host;
			server.ports_.push_back(parseNumber("port"));
		}
		else
			server.ports_.push_back(firstPart);
	}
	else
	{
		std::string host;
		while (peek().type != COLON)
			host += consumeValue("host").value;
		if (!hostname(host))
			throw std::runtime_error("Invalid host '" + host + "'");
		consume(COLON, ":");
		server.host_ = host;
		server.ports_.push_back(parseNumber("port"));
	}
	if (server.ports_.size() > 1 && server.ports_[0] == 80)
		server.ports_.erase(server.ports_.begin());
	if (server.ports_.back() < 1 || server.ports_.back() > 65535)
		throw std::runtime_error("Port must be between 1 and 65535");
	consume(SEMICOLON, ";");
}

void ConfigParser::parseServerNameDirective(ServerConfig& server)
{
	consume(WORD, "server_name");
	while (peek().type != SEMICOLON)
	{
		const std::string name = consumeValue("hostname").value;
		if (!hostname(name))
			throw std::runtime_error("Invalid hostname '" + name + "'");
		server.server_names_.push_back(name);
	}
	consume(SEMICOLON, ";");
}

void ConfigParser::parseRootDirective(ServerConfig* server,
									  LocationConfig* location)
{
	consume(WORD, "root");
	const std::string value = parsePathValue("root path");
	if (value.find('.') != std::string::npos)
		throw std::runtime_error("Invalid root path '" + value + "'");
	consume(SEMICOLON, ";");
	if (server)
		server->root_ = value;
	if (location)
		location->root_ = value;
}

void ConfigParser::parseIndexDirective(ServerConfig* server,
									   LocationConfig* location)
{
	consume(WORD, "index");
	std::vector<std::string> values;
	while (peek().type != SEMICOLON)
		values.push_back(consumeValue("filename").value);
	if (values.empty())
		throw std::runtime_error("Expected 'filename', got " +
								 describe(peek()));
	consume(SEMICOLON, ";");
	if (server)
		server->index_file_ = values;
	if (location)
		location->index_file_ = values;
}

void ConfigParser::parseClientMaxBodySizeDirective(ServerConfig* server,
												   LocationConfig* location)
{
	consume(WORD, "client_max_body_size");
	const int megabytes = parseNumber("body size");
	if (consumeValue("m").value != "m" || megabytes < 0)
		throw std::runtime_error("client_max_body_size requires megabytes");
	consume(SEMICOLON, ";");
	const std::size_t bytes = static_cast<std::size_t>(megabytes) * 1024 * 1024;
	if (server)
		server->client_max_body_size_ = bytes;
	if (location)
		location->client_max_body_size_ = bytes;
}

void ConfigParser::parseErrorPageDirective(ServerConfig* server,
										   LocationConfig* location)
{
	consume(WORD, "error_page");
	std::vector<int> codes;
	while (peek().type == NUMBER)
	{
		const int code = parseNumber("status code");
		if (code < 100 || code > 511)
			throw std::runtime_error("Invalid status code");
		codes.push_back(code);
	}
	const std::string value = parsePathValue("error page path");
	consume(SEMICOLON, ";");
	for (std::size_t i = 0; i < codes.size(); ++i)
	{
		if (server)
			server->error_pages_[codes[i]] = value;
		if (location)
			location->error_pages_[codes[i]] = value;
	}
}

void ConfigParser::parseMethodDirective(LocationConfig& location)
{
	consume(WORD, "method");
	location.allowed_methods_.clear();
	while (peek().type != SEMICOLON)
	{
		const std::string value = consumeValue("HTTP method").value;
		if (value != "GET" && value != "POST" && value != "DELETE" &&
			value != "HEAD")
			throw std::runtime_error("Unsupported HTTP method '" + value + "'");
		location.allowed_methods_.push_back(value);
	}
	consume(SEMICOLON, ";");
}

void ConfigParser::parseAutoindexDirective(LocationConfig& location)
{
	consume(WORD, "autoindex");
	const std::string value = consumeValue("'on' or 'off'").value;
	if (value != "on" && value != "off")
		throw std::runtime_error("Invalid autoindex value");
	location.autoindex_ = value == "on";
	consume(SEMICOLON, ";");
}

void ConfigParser::parseUploadDirDirective(LocationConfig& location)
{
	consume(WORD, "upload_dir");
	location.upload_store_ = parsePathValue("upload directory");
	location.upload_enabled_ = true;
	consume(SEMICOLON, ";");
}

void ConfigParser::parseReturnDirective(LocationConfig& location)
{
	consume(WORD, "return");
	int status = 302;
	if (peek().type == NUMBER)
		status = parseNumber("status code");
	if (status < 100 || status > 511)
		throw std::runtime_error("Invalid status code");
	const t_lexer_token& destination = consumeValue("redirect destination");
	if (destination.type != PATH && destination.type != URL)
		throw std::runtime_error("Return destination must be a path or URL");
	location.redirect_ = toString(status) + " " + destination.value;
	consume(SEMICOLON, ";");
}

void ConfigParser::parseCgiPassDirective(LocationConfig& location)
{
	consume(WORD, "cgi_pass");
	const std::string extension = consumeValue("extension").value;
	if (extension.size() < 2 || extension[0] != '.')
		throw std::runtime_error("Invalid CGI extension");
	for (std::size_t i = 1; i < extension.size(); ++i)
		if (!std::isalpha(static_cast<unsigned char>(extension[i])))
			throw std::runtime_error("Invalid CGI extension");

	const std::string executable = parsePathValue("CGI executable path");
	consume(SEMICOLON, ";");
	if (location.cgi_extensions_.find(extension) !=
		location.cgi_extensions_.end())
		throw std::runtime_error("Duplicate CGI extension '" + extension + "'");
	location.cgi_extensions_[extension] = executable;
}
