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
	stream << tokenTypeToString(token.type_) << " '" << token.value_ << "' at "
		   << token.line_ << ":" << token.column_;
	return stream.str();
}

bool word(const t_lexer_token& token, const std::string& value)
{
	return token.type_ == WORD && token.value_ == value;
}

int integer(const std::string& value)
{
	std::istringstream stream(value);
	int result;
	char extra;
	if ((stream >> result) == 0 || (stream >> extra) != 0)
	{
		throw std::runtime_error("Invalid number '" + value + "'");
	}
	return result;
}

bool hostname(const std::string& value)
{
	if (value.empty() || value[0] == '.' || value[value.size() - 1] == '.' ||
		value.find('*') != std::string::npos ||
		value.find('~') != std::string::npos)
	{
		return false;
	}
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		if (std::isalnum(static_cast<int>(value[i] == 0)) != 0 &&
			value[i] != '-' && value[i] != '.')
		{
			return false;
		}
	}
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

ConfigParser::ConfigParser(Lexer& lexer) : lexer_(lexer)
{
	lexer_.tokenize();
}

ConfigParser::~ConfigParser()
{
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
	if (lexer_.peek().type_ != type)
	{
		throw std::runtime_error("Expected '" + expected + "', got " +
								 describe(lexer_.peek()));
	}
	return lexer_.advance();
}

const t_lexer_token& ConfigParser::consumeValue(const std::string& expected)
{
	t_token value = lexer_.peek().type_;

	if (value != WORD && value != NUMBER && value != PATH && value != URL)
	{
		throw std::runtime_error("Expected '" + expected + "', got " +
								 describe(lexer_.peek()));
	}
	return lexer_.advance();
}

std::string ConfigParser::parsePathValue(const std::string& expected)
{
	t_lexer_token first = consumeValue(expected);
	if (first.type_ == WORD && first.value_.find('.') == std::string::npos)
	{
		first.type_ = PATH;
	}
	std::string result = first.value_;
	if (lexer_.peek().type_ == PATH &&
		lexer_.peek().column_ ==
			first.column_ + static_cast<int>(first.value_.size()))
	{
		result += lexer_.advance().value_;
	}
	if (!path(result))
	{
		throw std::runtime_error("Invalid path '" + result + "'");
	}
	return result;
}

int ConfigParser::parseNumber(const std::string& expected)
{
	return integer(consume(NUMBER, expected).value_);
}

void ConfigParser::rejectDuplicate(bool& seen, const std::string& directive)
{
	if (seen)
	{
		throw std::runtime_error("Duplicate directive '" + directive + "'");
	}
	seen = true;
}

std::vector<ServerConfig> ConfigParser::parseConfig()
{
	std::vector<ServerConfig> result;
	while (lexer_.peek().type_ != EOF_TOKEN)
	{
		result.push_back(parseServerBlock());
	}
	return result;
}

ServerConfig ConfigParser::parseServerBlock()
{
	if (!word(lexer_.peek(), "server"))
	{
		throw std::runtime_error("Expected 'server', got " +
								 describe(lexer_.peek()));
	}
	consume(WORD, "server");
	consume(BRACE, "{");
	ServerConfig server;
	bool root_seen = false;
	bool listen_seen = false;
	bool name_seen = false;
	bool index_seen = false;
	bool body_size_seen = false;

	while (lexer_.peek().type_ != CBRACE)
	{
		if (lexer_.peek().type_ == EOF_TOKEN)
		{
			throw std::runtime_error("Unclosed server block");
		}
		if (word(lexer_.peek(), "location"))
		{
			server.locations_.push_back(parseLocationBlock());
		}
		else if (word(lexer_.peek(), "root"))
		{
			rejectDuplicate(root_seen, "root");
			parseRootDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "listen"))
		{
			rejectDuplicate(listen_seen, "listen");
			parseListenDirective(server);
		}
		else if (word(lexer_.peek(), "server_name"))
		{
			rejectDuplicate(name_seen, "server_name");
			parseServerNameDirective(server);
		}
		else if (word(lexer_.peek(), "index"))
		{
			rejectDuplicate(index_seen, "index");
			parseIndexDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "client_max_body_size"))
		{
			rejectDuplicate(body_size_seen, "client_max_body_size");
			parseClientMaxBodySizeDirective(&server, NULL);
		}
		else if (word(lexer_.peek(), "error_page"))
		{
			parseErrorPageDirective(&server, NULL);
		}
		else
		{
			throw std::runtime_error("Unknown server directive " +
									 describe(lexer_.peek()));
		}
	}
	consume(CBRACE, "}");
	if (!root_seen)
	{
		throw std::runtime_error("Missing mandatory 'root' directive");
	}
	for (std::size_t i = 0; i < server.locations_.size(); ++i)
	{
		if (server.locations_[i].root_.empty())
		{
			server.locations_[i].root_ = server.root_;
		}
	}
	return server;
}

LocationConfig ConfigParser::parseLocationBlock()
{
	consume(WORD, "location");
	LocationConfig location;
	location.path_ = parsePathValue("location path");
	consume(BRACE, "{");

	bool root_seen = false;
	bool index_seen = false;
	bool body_size_seen = false;
	bool method_seen = false;
	bool autoindex_seen = false;
	bool upload_seen = false;
	bool return_seen = false;
	while (lexer_.peek().type_ != CBRACE)
	{
		if (lexer_.peek().type_ == EOF_TOKEN)
		{
			throw std::runtime_error("Unclosed location block");
		}
		if (word(peek(), "location"))
		{
			throw std::runtime_error("Nested location blocks are not allowed");
		}
		if (word(peek(), "method"))
		{
			rejectDuplicate(method_seen, "method");
			parseMethodDirective(location);
		}
		else if (word(peek(), "autoindex"))
		{
			rejectDuplicate(autoindex_seen, "autoindex");
			parseAutoindexDirective(location);
		}
		else if (word(peek(), "root"))
		{
			rejectDuplicate(root_seen, "root");
			parseRootDirective(NULL, &location);
		}
		else if (word(peek(), "index"))
		{
			rejectDuplicate(index_seen, "index");
			parseIndexDirective(NULL, &location);
		}
		else if (word(peek(), "client_max_body_size"))
		{
			rejectDuplicate(body_size_seen, "client_max_body_size");
			parseClientMaxBodySizeDirective(NULL, &location);
		}
		else if (word(peek(), "upload_dir"))
		{
			rejectDuplicate(upload_seen, "upload_dir");
			parseUploadDirDirective(location);
		}
		else if (word(peek(), "return"))
		{
			rejectDuplicate(return_seen, "return");
			parseReturnDirective(location);
		}
		else if (word(peek(), "error_page"))
		{
			parseErrorPageDirective(NULL, &location);
		}
		else if (word(peek(), "cgi_pass"))
		{
			parseCgiPassDirective(location);
		}
		else
		{
			throw std::runtime_error("Unknown location directive " +
									 describe(peek()));
		}
	}
	consume(CBRACE, "}");
	return location;
}

void ConfigParser::parseListenDirective(ServerConfig& server)
{
	consume(WORD, "listen");
	if (peek().type_ == NUMBER)
	{
		const int FIRST_PART = parseNumber("port or host");
		if (peek().type_ == WORD && peek().value_.size() > 1 &&
			peek().value_[0] == '.')
		{
			const std::string HOST =
				toString(FIRST_PART) + lexer_.advance().value_;
			if (!hostname(HOST))
			{
				throw std::runtime_error("Invalid host '" + HOST + "'");
			}
			consume(COLON, ":");
			server.host_ = HOST;
			server.ports_.push_back(parseNumber("port"));
		}
		else
		{
			server.ports_.push_back(FIRST_PART);
		}
	}
	else
	{
		std::string host;
		while (peek().type_ != COLON)
		{
			host += consumeValue("host").value_;
		}
		if (!hostname(host))
		{
			throw std::runtime_error("Invalid host '" + host + "'");
		}
		consume(COLON, ":");
		server.host_ = host;
		server.ports_.push_back(parseNumber("port"));
	}
	if (server.ports_.size() > 1 && server.ports_[0] == 80)
	{
		server.ports_.erase(server.ports_.begin());
	}
	if (server.ports_.back() < 1 || server.ports_.back() > 65535)
	{
		throw std::runtime_error("Port must be between 1 and 65535");
	}
	consume(SEMICOLON, ";");
}

void ConfigParser::parseServerNameDirective(ServerConfig& server)
{
	consume(WORD, "server_name");
	while (peek().type_ != SEMICOLON)
	{
		const std::string NAME = consumeValue("hostname").value_;
		if (!hostname(NAME))
		{
			throw std::runtime_error("Invalid hostname '" + NAME + "'");
		}
		server.server_names_.push_back(NAME);
	}
	consume(SEMICOLON, ";");
}

void ConfigParser::parseRootDirective(ServerConfig* server,
									  LocationConfig* location)
{
	consume(WORD, "root");
	const std::string VALUE = parsePathValue("root path");
	if (VALUE.find('.') != std::string::npos)
	{
		throw std::runtime_error("Invalid root path '" + VALUE + "'");
	}
	consume(SEMICOLON, ";");
	if (server != 0)
	{
		server->root_ = VALUE;
	}
	if (location != 0)
	{
		location->root_ = VALUE;
	}
}

void ConfigParser::parseIndexDirective(ServerConfig* server,
									   LocationConfig* location)
{
	consume(WORD, "index");
	std::vector<std::string> values;
	while (peek().type_ != SEMICOLON)
	{
		values.push_back(consumeValue("filename").value_);
	}
	if (values.empty())
	{
		throw std::runtime_error("Expected 'filename', got " +
								 describe(peek()));
	}
	consume(SEMICOLON, ";");
	if (server != 0)
	{
		server->index_file_ = values;
	}
	if (location != 0)
	{
		location->index_file_ = values;
	}
}

void ConfigParser::parseClientMaxBodySizeDirective(ServerConfig* server,
												   LocationConfig* location)
{
	consume(WORD, "client_max_body_size");
	const int MEGABYTES = parseNumber("body size");
	if (consumeValue("m").value_ != "m" || MEGABYTES < 0)
	{
		throw std::runtime_error("client_max_body_size requires megabytes");
	}
	consume(SEMICOLON, ";");
	const std::size_t BYTES = static_cast<std::size_t>(MEGABYTES) * 1024 * 1024;
	if (server != 0)
	{
		server->client_max_body_size_ = BYTES;
	}
	if (location != 0)
	{
		location->client_max_body_size_ = BYTES;
	}
}

void ConfigParser::parseErrorPageDirective(ServerConfig* server,
										   LocationConfig* location)
{
	consume(WORD, "error_page");
	std::vector<int> codes;
	while (peek().type_ == NUMBER)
	{
		const int CODE = parseNumber("status code");
		if (CODE < 100 || CODE > 511)
		{
			throw std::runtime_error("Invalid status code");
		}
		codes.push_back(CODE);
	}
	const std::string VALUE = parsePathValue("error page path");
	consume(SEMICOLON, ";");
	for (std::size_t i = 0; i < codes.size(); ++i)
	{
		if (server != 0)
		{
			server->error_pages_[codes[i]] = VALUE;
		}
		if (location != 0)
		{
			location->error_pages_[codes[i]] = VALUE;
		}
	}
}

void ConfigParser::parseMethodDirective(LocationConfig& location)
{
	consume(WORD, "method");
	location.allowed_methods_.clear();
	while (peek().type_ != SEMICOLON)
	{
		const std::string VALUE = consumeValue("HTTP method").value_;
		if (VALUE != "GET" && VALUE != "POST" && VALUE != "DELETE" &&
			VALUE != "UPDATE")
		{
			throw std::runtime_error("Unsupported HTTP method '" + VALUE + "'");
		}
		location.allowed_methods_.push_back(VALUE);
	}
	consume(SEMICOLON, ";");
}

void ConfigParser::parseAutoindexDirective(LocationConfig& location)
{
	consume(WORD, "autoindex");
	const std::string VALUE = consumeValue("'on' or 'off'").value_;
	if (VALUE != "on" && VALUE != "off")
	{
		throw std::runtime_error("Invalid autoindex value");
	}
	location.autoindex_ = VALUE == "on";
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
	if (peek().type_ == NUMBER)
	{
		status = parseNumber("status code");
	}
	if (status < 100 || status > 511)
	{
		throw std::runtime_error("Invalid status code");
	}
	const t_lexer_token& destination = consumeValue("redirect destination");
	if (destination.type_ != PATH && destination.type_ != URL)
	{
		throw std::runtime_error("Return destination must be a path or URL");
	}
	location.redirect_ = toString(status) + " " + destination.value_;
	consume(SEMICOLON, ";");
}

void ConfigParser::parseCgiPassDirective(LocationConfig& location)
{
	consume(WORD, "cgi_pass");
	const std::string EXTENSION = consumeValue("extension").value_;
	if (EXTENSION.size() < 2 || EXTENSION[0] != '.')
	{
		throw std::runtime_error("Invalid CGI extension");
	}
	for (std::size_t i = 1; i < EXTENSION.size(); ++i)
	{
		if (std::isalpha(static_cast<unsigned char>(EXTENSION[i])) == 0)
		{
			throw std::runtime_error("Invalid CGI extension");
		}
	}

	const std::string EXECUTABLE = parsePathValue("CGI executable path");
	consume(SEMICOLON, ";");
	if (location.cgi_extensions_.find(EXTENSION) !=
		location.cgi_extensions_.end())
	{
		throw std::runtime_error("Duplicate CGI extension '" + EXTENSION + "'");
	}
	location.cgi_extensions_[EXTENSION] = EXECUTABLE;
}
