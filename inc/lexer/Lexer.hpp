#ifndef LEXER_HPP
#define LEXER_HPP
#include <iostream>
#include <algorithm>
#include <ostream>
#include <vector>
#include <cctype>
#include <string>
#include <sstream>

typedef enum e_token
{
	WORD,
	NUMBER, // sequence of digits
	PATH,	// "/" followed by ALLOWED_CHAR*
	URL,	// scheme "://" + rest
	BRACE,
	CBRACE,
	SEMICOLON,
	COLON,
	EOF_TOKEN
} t_token;

typedef struct s_token
{
	std::string value;
	t_token type;
	int line;
	int column;
} t_lexer_token;

std::string tokenTypeToString(t_token type);

class Lexer
{
  private:
	std::vector<t_lexer_token> tokens_;
	std::string input_;
	std::size_t tokenIndex_;

	int pos_;
	int line_;
	int column_;

	char currentChar() const;
	char peekChar() const;
	void advanceChar();
	void addToken(t_token type, const std::string& value, int line, int column);
	void removeComments();
	void tokenizeWord();
	void tokenizeNumber();
	void tokenizePath();
	void tokenizeUrl();

  public:
	Lexer();
	Lexer(const std::string& input);
	virtual ~Lexer();

	static std::string errorMessage(char ch, int line, int column);

	void tokenize();
	const std::vector<t_lexer_token>& getTokens() const;
	const t_lexer_token& peek() const;
	const t_lexer_token& advance();
};

template <typename T> std::string toString(const T& value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::ostream& operator<<(std::ostream& out, const t_lexer_token& token);
std::ostream& operator<<(std::ostream& out, const Lexer& lexer);

#endif