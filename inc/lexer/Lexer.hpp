#ifndef LEXER_HPP
#define LEXER_HPP
#include <iostream>
#include <algorithm>
#include <ostream>
#include <vector>

typedef enum e_token
{
	WORD = 0,
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
	t_token type;
	std::string value;
	int line;
	int column;
} t_lexer_token;

std::string tokenTypeToString(t_token type);

class Lexer
{
  private:
	std::vector<t_lexer_token> _tokens;
	std::string _input;
	std::size_t _tokenIndex;

	int _pos;
	int _line;
	int _column;

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

	void tokenize();
	const std::vector<t_lexer_token>& getTokens() const;
	const t_lexer_token& peek() const;
	const t_lexer_token& advance();
	const t_lexer_token& expect(t_token type);
};
#endif

std::ostream& operator<<(std::ostream& out, const t_lexer_token& token);
std::ostream& operator<<(std::ostream& out, const Lexer& lexer);
