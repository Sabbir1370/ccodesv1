// src/parser/tokenizer.cpp
#include "tokenizer.h"
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <iostream>

// Helper: Convert token type to string (for debugging)
std::string Token::toString() const
{
    // In practice, you'd have a mapping function
    return "Token[" + lexeme + "] at " + std::to_string(line) + ":" + std::to_string(column);
}

// Keyword lookup table
static std::unordered_map<std::string, TokenType> createKeywordMap()
{
    return {
        {"auto", TokenType::KEYWORD_AUTO},
        {"break", TokenType::KEYWORD_BREAK},
        {"case", TokenType::KEYWORD_CASE},
        {"char", TokenType::KEYWORD_CHAR},
        {"const", TokenType::KEYWORD_CONST},
        {"continue", TokenType::KEYWORD_CONTINUE},
        {"default", TokenType::KEYWORD_DEFAULT},
        {"do", TokenType::KEYWORD_DO},
        {"double", TokenType::KEYWORD_DOUBLE},
        {"else", TokenType::KEYWORD_ELSE},
        {"enum", TokenType::KEYWORD_ENUM},
        {"extern", TokenType::KEYWORD_EXTERN},
        {"float", TokenType::KEYWORD_FLOAT},
        {"for", TokenType::KEYWORD_FOR},
        {"goto", TokenType::KEYWORD_GOTO},
        {"if", TokenType::KEYWORD_IF},
        {"int", TokenType::KEYWORD_INT},
        {"long", TokenType::KEYWORD_LONG},
        {"register", TokenType::KEYWORD_REGISTER},
        {"return", TokenType::KEYWORD_RETURN},
        {"short", TokenType::KEYWORD_SHORT},
        {"signed", TokenType::KEYWORD_SIGNED},
        {"sizeof", TokenType::KEYWORD_SIZEOF},
        {"static", TokenType::KEYWORD_STATIC},
        {"struct", TokenType::KEYWORD_STRUCT},
        {"switch", TokenType::KEYWORD_SWITCH},
        {"typedef", TokenType::KEYWORD_TYPEDEF},
        {"union", TokenType::KEYWORD_UNION},
        {"unsigned", TokenType::KEYWORD_UNSIGNED},
        {"void", TokenType::KEYWORD_VOID},
        {"volatile", TokenType::KEYWORD_VOLATILE},
        {"while", TokenType::KEYWORD_WHILE},
    };
}

// Static keyword map
static const auto keywordMap = createKeywordMap();

// ==================== Tokenizer Implementation ====================

Tokenizer::Tokenizer(const std::string &source)
    : source(source), position(0), line(1), column(1) {}

std::vector<Token> Tokenizer::tokenize()
{
    tokens.clear();
    position = 0;
    line = 1;
    column = 1;

    while (!isAtEnd())
    {
        scanToken();
    }

    addToken(TokenType::END_OF_FILE, "");
    return tokens;
}

// ==================== Character Helpers ====================

char Tokenizer::peek() const
{
    if (isAtEnd())
        return '\0';
    return source[position];
}

char Tokenizer::peekNext() const
{
    if (position + 1 >= source.length())
        return '\0';
    return source[position + 1];
}

char Tokenizer::advance()
{
    if (isAtEnd())
        return '\0';
    char c = source[position++];
    if (c == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }
    return c;
}

bool Tokenizer::match(char expected)
{
    if (isAtEnd() || source[position] != expected)
        return false;
    advance();
    return true;
}

bool Tokenizer::isAtEnd() const
{
    return position >= source.length();
}

// ==================== Token Management ====================

void Tokenizer::addToken(TokenType type, const std::string &lexeme)
{
    tokens.emplace_back(type, lexeme, line, column - lexeme.length());
}

void Tokenizer::errorToken(const std::string &message)
{
    tokens.emplace_back(TokenType::ERROR, message, line, column);
}

// ==================== Scanning Methods ====================

void Tokenizer::scanToken()
{
    char c = advance();
    // size_t startCol = column - 1; // Column where token started

    switch (c)
    {
    case '#':
        // Skip entire preprocessor line AND the newline
        while (peek() != '\n' && !isAtEnd())
        {
            advance(); // Skip preprocessor content
        }
        // Skip the newline character too
        if (peek() == '\n')
        {
            advance(); // This increments line counter
        }
        // Don't produce any token
        break;
        // Single-character tokens
    case '(':
        addToken(TokenType::PUNCT_LPAREN, "(");
        break;
    case ')':
        addToken(TokenType::PUNCT_RPAREN, ")");
        break;
    case '{':
        addToken(TokenType::PUNCT_LBRACE, "{");
        break;
    case '}':
        addToken(TokenType::PUNCT_RBRACE, "}");
        break;
    case '[':
        addToken(TokenType::PUNCT_LBRACKET, "[");
        break;
    case ']':
        addToken(TokenType::PUNCT_RBRACKET, "]");
        break;
    case ';':
        addToken(TokenType::PUNCT_SEMICOLON, ";");
        break;
    case ',':
        addToken(TokenType::PUNCT_COMMA, ",");
        break;
    case '.':
        addToken(TokenType::PUNCT_DOT, ".");
        break;
    case '?':
        addToken(TokenType::PUNCT_QUESTION, "?");
        break;
    case ':':
        addToken(TokenType::PUNCT_COLON, ":");
        break;

    // Operators that could be one or two chars
    case '+':
        if (match('+'))
            addToken(TokenType::OP_INCREMENT, "++");
        else if (match('='))
            addToken(TokenType::OP_PLUS_ASSIGN, "+=");
        else
            addToken(TokenType::OP_PLUS, "+");
        break;
    case '-':
        if (match('-'))
            addToken(TokenType::OP_DECREMENT, "--");
        else if (match('='))
            addToken(TokenType::OP_MINUS_ASSIGN, "-=");
        else if (match('>'))
            addToken(TokenType::PUNCT_ARROW, "->");
        else
            addToken(TokenType::OP_MINUS, "-");
        break;
    case '*':
        if (match('='))
            addToken(TokenType::OP_TIMES_ASSIGN, "*=");
        else
            addToken(TokenType::OP_STAR, "*");
        break;
    case '/':
        if (match('/'))
        {
            skipSingleLineComment();
        }
        else if (match('*'))
        {
            skipMultiLineComment();
        }
        else if (match('='))
        {
            addToken(TokenType::OP_DIV_ASSIGN, "/=");
        }
        else
        {
            addToken(TokenType::OP_SLASH, "/");
        }
        break;
    case '%':
        if (match('='))
            addToken(TokenType::OP_MOD_ASSIGN, "%=");
        else
            addToken(TokenType::OP_PERCENT, "%");
        break;
    case '=':
        if (match('='))
            addToken(TokenType::OP_EQ, "==");
        else
            addToken(TokenType::OP_ASSIGN, "=");
        break;
    case '!':
        if (match('='))
            addToken(TokenType::OP_NE, "!=");
        else
            addToken(TokenType::OP_LOGIC_NOT, "!");
        break;
    case '<':
        if (match('<'))
        {
            if (match('='))
                addToken(TokenType::OP_SHL_ASSIGN, "<<=");
            else
                addToken(TokenType::OP_SHL, "<<");
        }
        else if (match('='))
        {
            addToken(TokenType::OP_LE, "<=");
        }
        else
        {
            addToken(TokenType::OP_LT, "<");
        }
        break;
    case '>':
        if (match('>'))
        {
            if (match('='))
                addToken(TokenType::OP_SHR_ASSIGN, ">>=");
            else
                addToken(TokenType::OP_SHR, ">>");
        }
        else if (match('='))
        {
            addToken(TokenType::OP_GE, ">=");
        }
        else
        {
            addToken(TokenType::OP_GT, ">");
        }
        break;
    case '&':
        if (match('&'))
            addToken(TokenType::OP_LOGIC_AND, "&&");
        else if (match('='))
            addToken(TokenType::OP_AND_ASSIGN, "&=");
        else
            addToken(TokenType::OP_BIT_AND, "&");
        break;
    case '|':
        if (match('|'))
            addToken(TokenType::OP_LOGIC_OR, "||");
        else if (match('='))
            addToken(TokenType::OP_OR_ASSIGN, "|=");
        else
            addToken(TokenType::OP_BIT_OR, "|");
        break;
    case '^':
        if (match('='))
            addToken(TokenType::OP_XOR_ASSIGN, "^=");
        else
            addToken(TokenType::OP_BIT_XOR, "^");
        break;
    case '~':
        addToken(TokenType::OP_BIT_NOT, "~");
        break;

    // String literals
    case '"':
        scanString('"');
        break;
    case '\'':
        scanChar();
        break;

    // Whitespace
    case ' ':
    case '\t':
    case '\r':
        // Ignore whitespace
        break;
    case '\n':
        // Newline already handled in advance()
        break;

    default:
        if (isDigit(c))
        {
            scanNumber();
        }
        else if (isAlpha(c))
        {
            scanIdentifierOrKeyword();
        }
        else
        {
            // Unknown character
            std::string msg = "Unexpected character: '";
            msg += c;
            msg += "'";
            errorToken(msg);
        }
        break;
    }
}

// ==================== Comment Handling ====================

void Tokenizer::skipSingleLineComment()
{
    // Consume until newline or EOF
    while (peek() != '\n' && !isAtEnd())
    {
        advance();
    }
}

void Tokenizer::skipMultiLineComment()
{
    // Consume until */ or EOF
    while (!isAtEnd())
    {
        if (peek() == '*' && peekNext() == '/')
        {
            advance(); // *
            advance(); // /
            return;
        }
        advance();
    }

    // If we get here, comment wasn't closed
    errorToken("Unclosed multi-line comment");
}

// ==================== Identifier & Keyword ====================

void Tokenizer::scanIdentifierOrKeyword()
{
    size_t start = position - 1; // Position of first character
    while (isAlphaNumeric(peek()))
    {
        advance();
    }

    std::string text = source.substr(start, position - start);

    // Check if it's a keyword
    auto it = keywordMap.find(text);
    if (it != keywordMap.end())
    {
        addToken(it->second, text);
    }
    else
    {
        addToken(TokenType::IDENTIFIER, text);
    }
}

// ==================== Numbers ====================

void Tokenizer::scanNumber()
{
    // For Phase B: simple integer detection
    // You can extend this for floats, hex, octal later
    while (isDigit(peek()))
    {
        advance();
    }

    // Check for float (optional for Phase B)
    if (peek() == '.' && isDigit(peekNext()))
    {
        advance(); // Consume '.'
        while (isDigit(peek()))
        {
            advance();
        }
        // Could check for exponent notation here
    }

    // In Phase B, we can treat all numbers as LITERAL_INT for simplicity
    size_t start = position - 1;
    while (start > 0 && (isDigit(source[start - 1]) || source[start - 1] == '.'))
    {
        start--;
    }

    std::string numText = source.substr(start, position - start);
    addToken(TokenType::LITERAL_INT, numText);
}

// ==================== Strings & Characters ====================

void Tokenizer::scanString(char delimiter)
{
    std::string value;
    value += delimiter;

    while (!isAtEnd() && peek() != delimiter)
    {
        if (peek() == '\\')
        {
            advance(); // Skip backslash
            // Handle escape sequences (simplified)
            if (!isAtEnd())
            {
                value += '\\';
                value += advance();
            }
        }
        else
        {
            value += advance();
        }
    }

    if (isAtEnd())
    {
        errorToken("Unterminated string literal");
        return;
    }

    advance(); // Consume closing delimiter
    value += delimiter;

    TokenType type = (delimiter == '"') ? TokenType::LITERAL_STRING : TokenType::LITERAL_CHAR;
    addToken(type, value);
}

void Tokenizer::scanChar()
{
    // Simplified char literal handling
    scanString('\'');
}

// ==================== Character Classification ====================

bool Tokenizer::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool Tokenizer::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Tokenizer::isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

bool Tokenizer::isHexDigit(char c)
{
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// ==================== Debug Helpers ====================

std::string Tokenizer::getCurrentLineContext() const
{
    // Find start of current line
    size_t lineStart = position;
    while (lineStart > 0 && source[lineStart - 1] != '\n')
    {
        lineStart--;
    }

    // Find end of current line
    size_t lineEnd = position;
    while (lineEnd < source.length() && source[lineEnd] != '\n')
    {
        lineEnd++;
    }

    return source.substr(lineStart, lineEnd - lineStart);
}