#include "lexer/Lexer.hpp"
#include <cctype>

std::string tokenTypeToString(t_token type)
{
	switch (type)
	{
		case WORD:
			return "WORD";
		case NUMBER:
			return "NUMBER";
		case PATH:
			return "PATH";
		case URL:
			return "URL";
		case BRACE:
			return "BRACE";
		case CBRACE:
			return "CBRACE";
		case SEMICOLON:
			return "SEMICOLON";
		case COLON:
			return "COLON";
		case EOF_TOKEN:
			return "EOF_TOKEN";
		default:
			return "UNKNOWN";
	}
}

Lexer::Lexer() : _input(), _tokenIndex(0), _pos(0), _line(1), _column(1)
{
}

Lexer::Lexer(const std::string& input)
	: _input(input), _tokenIndex(0), _pos(0), _line(1), _column(1)
{
}

Lexer::~Lexer()
{
}

char Lexer::currentChar() const
{
	if (static_cast<std::size_t>(_pos) >= _input.size())
		return '\0';
	return _input[_pos];
}

char Lexer::peekChar() const
{
	std::size_t nextPos = static_cast<std::size_t>(_pos) + 1;
	if (nextPos >= _input.size())
		return '\0';
	return _input[nextPos];
}

void Lexer::advanceChar()
{
	if (currentChar() == '\0')
		return;
	if (currentChar() == '\n')
	{
		_line++;
		_column = 1;
	}
	else
		_column++;
	_pos++;
}

void Lexer::addToken(t_token type,
					 const std::string& value,
					 int line,
					 int column)
{
	t_lexer_token token;
	token.type = type;
	token.value = value;
	token.line = line;
	token.column = column;
	_tokens.push_back(token);
}

void Lexer::removeComments()
{
	while (currentChar() != '\0' && currentChar() != '\n')
		advanceChar();
	if (currentChar() == '\n')
		advanceChar();
}

void Lexer::tokenizeWord()
{
	std::size_t startPos = static_cast<std::size_t>(_pos);
	int startLine = _line;
	int startColumn = _column;
	std::size_t schemePos = startPos;

	while (schemePos < _input.size() &&
		   std::isalpha(static_cast<unsigned char>(_input[schemePos])) != 0)
		schemePos++;
	if (schemePos + 2 < _input.size() && _input[schemePos] == ':' &&
		_input[schemePos + 1] == '/' && _input[schemePos + 2] == '/')
	{
		std::size_t endPos = schemePos + 3;
		while (endPos < _input.size() &&
			   !std::isspace(static_cast<unsigned char>(_input[endPos])) &&
			   _input[endPos] != ';' && _input[endPos] != '{' &&
			   _input[endPos] != '}')
			endPos++;
		std::string value = _input.substr(startPos, endPos - startPos);
		while (static_cast<std::size_t>(_pos) < endPos)
			advanceChar();
		addToken(URL, value, startLine, startColumn);
		return;
	}

	while (currentChar() != '\0')
	{
		char ch = currentChar();
		if (std::isspace(static_cast<unsigned char>(ch)) || ch == ';' ||
			ch == '{' || ch == '}' || ch == ':' || ch == '/')
			break;
		advanceChar();
	}
	std::string value =
		_input.substr(startPos, static_cast<std::size_t>(_pos) - startPos);
	addToken(WORD, value, startLine, startColumn);
}

void Lexer::tokenizeNumber()
{
	std::size_t startPos = static_cast<std::size_t>(_pos);
	int startLine = _line;
	int startColumn = _column;

	while (currentChar() != '\0' &&
		   std::isdigit(static_cast<unsigned char>(currentChar())) != 0)
		advanceChar();
	std::string value =
		_input.substr(startPos, static_cast<std::size_t>(_pos) - startPos);
	addToken(NUMBER, value, startLine, startColumn);
}

void Lexer::tokenizePath()
{
	std::size_t startPos = static_cast<std::size_t>(_pos);
	int startLine = _line;
	int startColumn = _column;

	while (currentChar() != '\0' &&
		   !std::isspace(static_cast<unsigned char>(currentChar())) &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
		advanceChar();
	std::string value =
		_input.substr(startPos, static_cast<std::size_t>(_pos) - startPos);
	addToken(PATH, value, startLine, startColumn);
}

void Lexer::tokenizeUrl()
{
	std::size_t startPos = static_cast<std::size_t>(_pos);
	int startLine = _line;
	int startColumn = _column;

	while (currentChar() != '\0' &&
		   !std::isspace(static_cast<unsigned char>(currentChar())) &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
		advanceChar();
	std::string value =
		_input.substr(startPos, static_cast<std::size_t>(_pos) - startPos);
	addToken(URL, value, startLine, startColumn);
}

void Lexer::tokenize()
{
	_tokens.clear();
	_pos = 0;
	_line = 1;
	_column = 1;
	_tokenIndex = 0;

	while (currentChar() != '\0')
	{
		char ch = currentChar();
		if (std::isspace(static_cast<unsigned char>(ch)) != 0)
		{
			advanceChar();
			continue;
		}
		if (ch == '#')
		{
			removeComments();
			continue;
		}
		if (ch == '{')
		{
			addToken(BRACE, std::string(1, ch), _line, _column);
			advanceChar();
			continue;
		}
		if (ch == '}')
		{
			addToken(CBRACE, std::string(1, ch), _line, _column);
			advanceChar();
			continue;
		}
		if (ch == ';')
		{
			addToken(SEMICOLON, std::string(1, ch), _line, _column);
			advanceChar();
			continue;
		}
		if (ch == ':')
		{
			addToken(COLON, std::string(1, ch), _line, _column);
			advanceChar();
			continue;
		}
		if (ch == '/')
		{
			tokenizePath();
			continue;
		}
		if (std::isdigit(static_cast<unsigned char>(ch)) != 0)
		{
			tokenizeNumber();
			continue;
		}
		if (std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_' ||
			ch == '-' || ch == '.')
		{
			tokenizeWord();
			continue;
		}
		advanceChar();
	}
	addToken(EOF_TOKEN, "", _line, _column);
}

const std::vector<t_lexer_token>& Lexer::getTokens() const
{
	return _tokens;
}

const t_lexer_token& Lexer::peek() const
{
	static const t_lexer_token eofToken = {EOF_TOKEN, "", 0, 0};
	if (_tokenIndex < _tokens.size())
		return _tokens[_tokenIndex];
	return eofToken;
}

const t_lexer_token& Lexer::advance()
{
	static const t_lexer_token eofToken = {EOF_TOKEN, "", 0, 0};
	if (_tokenIndex < _tokens.size())
	{
		const t_lexer_token& token = _tokens[_tokenIndex++];
		return token;
	}
	return eofToken;
}

const t_lexer_token& Lexer::expect(t_token type)
{
	static const t_lexer_token eofToken = {EOF_TOKEN, "", 0, 0};
	if (_tokenIndex < _tokens.size() && _tokens[_tokenIndex].type == type)
		return _tokens[_tokenIndex++];
	return eofToken;
}

std::ostream& operator<<(std::ostream& out, const t_lexer_token& token)
{
	out << tokenTypeToString(token.type) << ": \"" << token.value << "\"";
	if (token.line > 0 || token.column > 0)
		out << " @(" << token.line << ":" << token.column << ")";
	return out;
}

std::ostream& operator<<(std::ostream& out, const Lexer& lexer)
{
	for (std::vector<t_lexer_token>::const_iterator it =
			 lexer.getTokens().begin();
		 it != lexer.getTokens().end();
		 ++it)
		out << *it << "\n";
	return (out);
}
