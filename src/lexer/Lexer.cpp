#include "lexer/Lexer.hpp"

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

std::string Lexer::errorMessage(char ch, int line, int column)
{
	std::string value(1, ch);
	if (ch == '\0')
		value = "\0";
	return "Unexpected character '" + value + "' at line " + toString(line) +
		   ", column " + toString(column);
}

Lexer::Lexer() : input_(), tokenIndex_(0), pos_(0), line_(1), column_(1)
{
}

Lexer::Lexer(const std::string& input)
	: input_(input), tokenIndex_(0), pos_(0), line_(1), column_(1)
{
}

Lexer::~Lexer()
{
}

char Lexer::currentChar() const
{
	if (static_cast<std::size_t>(pos_) >= input_.size())
		return '\0';
	return input_[pos_];
}

char Lexer::peekChar() const
{
	std::size_t nextPos = static_cast<std::size_t>(pos_) + 1;
	if (nextPos >= input_.size())
		return '\0';
	return input_[nextPos];
}

void Lexer::advanceChar()
{
	if (currentChar() == '\0')
		return;
	if (currentChar() == '\n')
	{
		line_++;
		column_ = 1;
	}
	else
		column_++;
	pos_++;
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
	tokens_.push_back(token);
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
	std::size_t startPos = static_cast<std::size_t>(pos_);
	int startLine = line_;
	int startColumn = column_;
	std::size_t schemePos = startPos;

	while (schemePos < input_.size() &&
		   std::isalpha(static_cast<unsigned char>(input_[schemePos])) != 0)
		schemePos++;
	if (schemePos + 2 < input_.size() && input_[schemePos] == ':' &&
		input_[schemePos + 1] == '/' && input_[schemePos + 2] == '/')
	{
		std::size_t endPos = schemePos + 3;
		while (endPos < input_.size() &&
			   !std::isspace(static_cast<unsigned char>(input_[endPos])) &&
			   input_[endPos] != ';' && input_[endPos] != '{' &&
			   input_[endPos] != '}')
			endPos++;
		std::string value = input_.substr(startPos, endPos - startPos);
		while (static_cast<std::size_t>(pos_) < endPos)
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
		input_.substr(startPos, static_cast<std::size_t>(pos_) - startPos);
	addToken(WORD, value, startLine, startColumn);
}

void Lexer::tokenizeNumber()
{
	std::size_t startPos = static_cast<std::size_t>(pos_);
	int startLine = line_;
	int startColumn = column_;

	while (currentChar() != '\0' &&
		   std::isdigit(static_cast<unsigned char>(currentChar())) != 0)
		advanceChar();
	std::string value =
		input_.substr(startPos, static_cast<std::size_t>(pos_) - startPos);
	addToken(NUMBER, value, startLine, startColumn);
}

void Lexer::tokenizePath()
{
	std::size_t startPos = static_cast<std::size_t>(pos_);
	int startLine = line_;
	int startColumn = column_;

	while (currentChar() != '\0' &&
		   !std::isspace(static_cast<unsigned char>(currentChar())) &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
		advanceChar();
	std::string value =
		input_.substr(startPos, static_cast<std::size_t>(pos_) - startPos);
	addToken(PATH, value, startLine, startColumn);
}

void Lexer::tokenizeUrl()
{
	std::size_t startPos = static_cast<std::size_t>(pos_);
	int startLine = line_;
	int startColumn = column_;

	while (currentChar() != '\0' &&
		   !std::isspace(static_cast<unsigned char>(currentChar())) &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
		advanceChar();
	std::string value =
		input_.substr(startPos, static_cast<std::size_t>(pos_) - startPos);
	addToken(URL, value, startLine, startColumn);
}

void Lexer::tokenize()
{
	tokens_.clear();
	pos_ = 0;
	line_ = 1;
	column_ = 1;
	tokenIndex_ = 0;

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
			addToken(BRACE, std::string(1, ch), line_, column_);
			advanceChar();
			continue;
		}
		if (ch == '}')
		{
			addToken(CBRACE, std::string(1, ch), line_, column_);
			advanceChar();
			continue;
		}
		if (ch == ';')
		{
			addToken(SEMICOLON, std::string(1, ch), line_, column_);
			advanceChar();
			continue;
		}
		if (ch == ':')
		{
			addToken(COLON, std::string(1, ch), line_, column_);
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
		throw std::runtime_error(errorMessage(ch, line_, column_));
	}
	addToken(EOF_TOKEN, "", line_, column_);
}

const std::vector<t_lexer_token>& Lexer::getTokens() const
{
	return tokens_;
}

const t_lexer_token& Lexer::peek() const
{
	static const t_lexer_token eofToken = {"", EOF_TOKEN, 0, 0};
	if (tokenIndex_ < tokens_.size())
		return tokens_[tokenIndex_];
	return eofToken;
}

const t_lexer_token& Lexer::advance()
{
	static const t_lexer_token eofToken = {"", EOF_TOKEN, 0, 0};
	if (tokenIndex_ < tokens_.size())
	{
		const t_lexer_token& token = tokens_[tokenIndex_++];
		return token;
	}
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
