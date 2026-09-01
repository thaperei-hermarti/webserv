#include "lexer/Lexer.hpp"

#include <gtest/gtest.h>
#include <sstream>

TEST(LexerTest, TokenizesServerBlockGrammar)
{
	Lexer lexer("server { listen 8080; root /var/www; }");
	lexer.tokenize();
	const std::vector<t_lexer_token>& tokens = lexer.getTokens();

	ASSERT_EQ(10u, tokens.size());
	EXPECT_EQ(WORD, tokens[0].type);
	EXPECT_EQ("server", tokens[0].value);
	EXPECT_EQ(BRACE, tokens[1].type);
	EXPECT_EQ("{", tokens[1].value);
	EXPECT_EQ(WORD, tokens[2].type);
	EXPECT_EQ("listen", tokens[2].value);
	EXPECT_EQ(NUMBER, tokens[3].type);
	EXPECT_EQ("8080", tokens[3].value);
	EXPECT_EQ(SEMICOLON, tokens[4].type);
	EXPECT_EQ(";", tokens[4].value);
	EXPECT_EQ(WORD, tokens[5].type);
	EXPECT_EQ("root", tokens[5].value);
	EXPECT_EQ(PATH, tokens[6].type);
	EXPECT_EQ("/var/www", tokens[6].value);
	EXPECT_EQ(SEMICOLON, tokens[7].type);
	EXPECT_EQ(CBRACE, tokens[8].type);
	EXPECT_EQ("}", tokens[8].value);
	EXPECT_EQ(EOF_TOKEN, tokens.back().type);
}

TEST(LexerTest, ExposesReadableTokenTypeNames)
{
	Lexer lexer("server { listen 8080; }");
	lexer.tokenize();
	const std::vector<t_lexer_token>& tokens = lexer.getTokens();

	EXPECT_EQ("WORD", tokenTypeToString(tokens[0].type));
	EXPECT_EQ("BRACE", tokenTypeToString(tokens[1].type));
	EXPECT_EQ("NUMBER", tokenTypeToString(tokens[3].type));

	std::ostringstream stream;
	stream << tokens[0];
	EXPECT_NE(std::string::npos, stream.str().find("WORD"));
	EXPECT_NE(std::string::npos, stream.str().find("server"));
}

TEST(LexerTest, RecognizesPathsAndUrls)
{
	Lexer lexer("return \
			https://example.com:8080/path; method GET POST DELETE;");
	lexer.tokenize();
	const std::vector<t_lexer_token>& tokens = lexer.getTokens();

	ASSERT_GE(tokens.size(), 9u);
	EXPECT_EQ(WORD, tokens[0].type);
	EXPECT_EQ("return", tokens[0].value);
	EXPECT_EQ(URL, tokens[1].type);
	EXPECT_EQ("https://example.com:8080/path", tokens[1].value);
	EXPECT_EQ(SEMICOLON, tokens[2].type);
	EXPECT_EQ(WORD, tokens[3].type);
	EXPECT_EQ("method", tokens[3].value);
	EXPECT_EQ(WORD, tokens[4].type);
	EXPECT_EQ("GET", tokens[4].value);
	EXPECT_EQ(WORD, tokens[5].type);
	EXPECT_EQ("POST", tokens[5].value);
	EXPECT_EQ(WORD, tokens[6].type);
	EXPECT_EQ("DELETE", tokens[6].value);
	EXPECT_EQ(SEMICOLON, tokens[7].type);
	EXPECT_EQ(EOF_TOKEN, tokens.back().type);
}

TEST(LexerTest, IgnoresCommentsAndWhitespace)
{
	Lexer lexer("server { # comment\nroot /var/www; # trailing comment\n}");
	lexer.tokenize();
	const std::vector<t_lexer_token>& tokens = lexer.getTokens();

	ASSERT_GE(tokens.size(), 7u);
	EXPECT_EQ(WORD, tokens[0].type);
	EXPECT_EQ("server", tokens[0].value);
	EXPECT_EQ(BRACE, tokens[1].type);
	EXPECT_EQ(WORD, tokens[2].type);
	EXPECT_EQ("root", tokens[2].value);
	EXPECT_EQ(PATH, tokens[3].type);
	EXPECT_EQ("/var/www", tokens[3].value);
	EXPECT_EQ(SEMICOLON, tokens[4].type);
	EXPECT_EQ(CBRACE, tokens[5].type);
	EXPECT_EQ(EOF_TOKEN, tokens.back().type);
}

TEST(LexerTest, ProducesDetailedCharacterErrorMessage)
{
	EXPECT_EQ("Unexpected character '@' at line 2, column 4",
			  Lexer::errorMessage('@', 2, 4));
}
