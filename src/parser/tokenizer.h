// src/include/tokenizer.h
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <cstddef>

// Token types for C source code
enum class TokenType
{
    // Keywords
    KEYWORD_AUTO,
    KEYWORD_BREAK,
    KEYWORD_CASE,
    KEYWORD_CHAR,
    KEYWORD_CONST,
    KEYWORD_CONTINUE,
    KEYWORD_DEFAULT,
    KEYWORD_DO,
    KEYWORD_DOUBLE,
    KEYWORD_ELSE,
    KEYWORD_ENUM,
    KEYWORD_EXTERN,
    KEYWORD_FLOAT,
    KEYWORD_FOR,
    KEYWORD_GOTO,
    KEYWORD_IF,
    KEYWORD_INT,
    KEYWORD_LONG,
    KEYWORD_REGISTER,
    KEYWORD_RETURN,
    KEYWORD_SHORT,
    KEYWORD_SIGNED,
    KEYWORD_SIZEOF,
    KEYWORD_STATIC,
    KEYWORD_STRUCT,
    KEYWORD_SWITCH,
    KEYWORD_TYPEDEF,
    KEYWORD_UNION,
    KEYWORD_UNSIGNED,
    KEYWORD_VOID,
    KEYWORD_VOLATILE,
    KEYWORD_WHILE,

    // Identifiers and literals
    IDENTIFIER,
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_CHAR,
    LITERAL_STRING,

    // Operators
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,
    OP_PERCENT,
    OP_ASSIGN,
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_TIMES_ASSIGN,
    OP_DIV_ASSIGN,
    OP_MOD_ASSIGN,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_LOGIC_AND,
    OP_LOGIC_OR,
    OP_LOGIC_NOT,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_NOT,
    OP_SHL,
    OP_SHR,
    OP_SHL_ASSIGN,
    OP_SHR_ASSIGN,
    OP_AND_ASSIGN,
    OP_OR_ASSIGN,
    OP_XOR_ASSIGN,
    OP_INCREMENT,
    OP_DECREMENT,

    // Punctuators
    PUNCT_SEMICOLON,
    PUNCT_COMMA,
    PUNCT_DOT,
    PUNCT_ARROW,
    PUNCT_LPAREN,
    PUNCT_RPAREN,
    PUNCT_LBRACKET,
    PUNCT_RBRACKET,
    PUNCT_LBRACE,
    PUNCT_RBRACE,
    PUNCT_COLON,
    PUNCT_QUESTION,

    // Special
    END_OF_FILE,
    ERROR
};

// Single token structure
struct Token
{
    TokenType type;
    std::string lexeme; // Raw text of the token
    int line;           // Line number (1-based)
    int column;         // Column number (1-based)

    // Constructor
    Token(TokenType t, std::string lex, int ln, int col)
        : type(t), lexeme(std::move(lex)), line(ln), column(col) {}

    // For debugging
    std::string toString() const;
};

class Tokenizer
{
public:
    explicit Tokenizer(const std::string &source);

    // Main tokenization method
    std::vector<Token> tokenize();

    // Get the source code (optional)
    const std::string &getSource() const { return source; }

    // For error reporting
    std::string getCurrentLineContext() const;

private:
    std::string source;
    size_t position; // Current index in source
    size_t line;     // Current line number (1-based)
    size_t column;   // Current column number (1-based)
    std::vector<Token> tokens;

    // Character helpers
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    bool isAtEnd() const;

    // Token creation
    void addToken(TokenType type, const std::string &lexeme);

    // Token scanning
    void scanToken();
    void scanIdentifierOrKeyword();
    void scanNumber();
    void scanString(char delimiter);
    void scanChar();

    // Comment handling
    void skipSingleLineComment();
    void skipMultiLineComment();

    // Helper checks
    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
    static bool isHexDigit(char c);

    // Error token
    void errorToken(const std::string &message);
};

#endif // TOKENIZER_H