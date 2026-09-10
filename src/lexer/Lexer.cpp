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
	{
		value = "\\0";
	}
	return "Unexpected character '" + value + "' at line " + toString(line) +
		   ", column " + toString(column);
}

Lexer::Lexer() : token_index_(0), pos_(0), line_(1), column_(1)
{
}

Lexer::Lexer(const std::string& input)
	: input_(input), token_index_(0), pos_(0), line_(1), column_(1)
{
}

Lexer::~Lexer()
{
}

char Lexer::currentChar() const
{
	if (static_cast<std::size_t>(pos_) >= input_.size())
	{
		return '\0';
	}
	return input_[pos_];
}

char Lexer::peekChar() const
{
	std::size_t next_pos = static_cast<std::size_t>(pos_) + 1;
	if (next_pos >= input_.size())
	{
		return '\0';
	}
	return input_[next_pos];
}

void Lexer::advanceChar()
{
	if (currentChar() == '\0')
	{
		return;
	}
	if (currentChar() == '\n')
	{
		line_++;
		column_ = 1;
	}
	else
	{
		column_++;
	}
	pos_++;
}

void Lexer::addToken(t_token type,
					 const std::string& value,
					 int line,
					 int column)
{
	t_lexer_token token;
	token.type_ = type;
	token.value_ = value;
	token.line_ = line;
	token.column_ = column;
	tokens_.push_back(token);
}

void Lexer::removeComments()
{
	while (currentChar() != '\0' && currentChar() != '\n')
	{
		advanceChar();
	}
	if (currentChar() == '\n')
	{
		advanceChar();
	}
}

void Lexer::tokenizeWord()
{
	std::size_t start_pos = static_cast<std::size_t>(pos_);
	int start_line = line_;
	int start_column = column_;
	std::size_t scheme_pos = start_pos;

	while (scheme_pos < input_.size() &&
		   std::isalpha(static_cast<unsigned char>(input_[scheme_pos])) != 0)
	{
		scheme_pos++;
	}
	if (scheme_pos + 2 < input_.size() && input_[scheme_pos] == ':' &&
		input_[scheme_pos + 1] == '/' && input_[scheme_pos + 2] == '/')
	{
		std::size_t end_pos = scheme_pos + 3;
		while (end_pos < input_.size() &&
			   std::isspace(static_cast<unsigned char>(input_[end_pos])) == 0 &&
			   input_[end_pos] != ';' && input_[end_pos] != '{' &&
			   input_[end_pos] != '}')
		{
			end_pos++;
		}
		std::string value = input_.substr(start_pos, end_pos - start_pos);
		while (static_cast<std::size_t>(pos_) < end_pos)
		{
			advanceChar();
		}
		addToken(URL, value, start_line, start_column);
		return;
	}

	while (currentChar() != '\0')
	{
		char ch = currentChar();
		if (std::isspace(static_cast<unsigned char>(ch)) != 0 || ch == ';' ||
			ch == '{' || ch == '}' || ch == ':' || ch == '/')
		{
			break;
		}
		advanceChar();
	}
	std::string value =
		input_.substr(start_pos, static_cast<std::size_t>(pos_) - start_pos);
	addToken(WORD, value, start_line, start_column);
}

void Lexer::tokenizeNumber()
{
	std::size_t start_pos = static_cast<std::size_t>(pos_);
	int start_line = line_;
	int start_column = column_;

	while (currentChar() != '\0' &&
		   std::isdigit(static_cast<unsigned char>(currentChar())) != 0)
	{
		advanceChar();
	}
	std::string value =
		input_.substr(start_pos, static_cast<std::size_t>(pos_) - start_pos);
	addToken(NUMBER, value, start_line, start_column);
}

void Lexer::tokenizePath()
{
	std::size_t start_pos = static_cast<std::size_t>(pos_);
	int start_line = line_;
	int start_column = column_;

	while (currentChar() != '\0' &&
		   std::isspace(static_cast<unsigned char>(currentChar())) == 0 &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
	{
		advanceChar();
	}
	std::string value =
		input_.substr(start_pos, static_cast<std::size_t>(pos_) - start_pos);
	addToken(PATH, value, start_line, start_column);
}

void Lexer::tokenizeUrl()
{
	std::size_t start_pos = static_cast<std::size_t>(pos_);
	int start_line = line_;
	int start_column = column_;

	while (currentChar() != '\0' &&
		   std::isspace(static_cast<unsigned char>(currentChar())) == 0 &&
		   currentChar() != ';' && currentChar() != '{' && currentChar() != '}')
	{
		advanceChar();
	}
	std::string value =
		input_.substr(start_pos, static_cast<std::size_t>(pos_) - start_pos);
	addToken(URL, value, start_line, start_column);
}

void Lexer::tokenize()
{
	tokens_.clear();
	pos_ = 0;
	line_ = 1;
	column_ = 1;
	token_index_ = 0;

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
	static const t_lexer_token EOF_T = {"", EOF_TOKEN, 0, 0};
	if (token_index_ < tokens_.size())
	{
		return tokens_[token_index_];
	}
	return EOF_T;
}

const t_lexer_token& Lexer::advance()
{
	static const t_lexer_token EOF_T = {"", EOF_TOKEN, 0, 0};
	if (token_index_ < tokens_.size())
	{
		const t_lexer_token& token = tokens_[token_index_++];
		return token;
	}
	return EOF_T;
}

std::ostream& operator<<(std::ostream& out, const t_lexer_token& token)
{
	out << tokenTypeToString(token.type_) << ": \"" << token.value_ << "\"";
	if (token.line_ > 0 || token.column_ > 0)
	{
		out << " @(" << token.line_ << ":" << token.column_ << ")";
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const Lexer& lexer)
{
	for (std::vector<t_lexer_token>::const_iterator it =
			 lexer.getTokens().begin();
		 it != lexer.getTokens().end();
		 ++it)
	{
		out << *it << "\n";
	}
	return (out);
}
