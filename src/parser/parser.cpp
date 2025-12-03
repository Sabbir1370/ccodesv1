#include "parser.h"
#include <iostream>

// ==================== Helper Methods ====================

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens) {}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

const Token &Parser::peek() const
{
    return tokens[current];
}

const Token &Parser::previous() const
{
    return tokens[current - 1];
}

const Token &Parser::advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return false;
    return peek().type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &errorMessage)
{
    if (check(type))
        return advance();

    error(peek(), errorMessage);
    return Token{TokenType::ERROR, "", 0, 0};
}

void Parser::error(const Token &token, const std::string &message)
{
    std::cerr << "[Line " << token.line << "] Error: " << message << std::endl;
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd())
    {
        if (previous().type == TokenType::PUNCT_SEMICOLON)
            return;

        switch (peek().type)
        {
        case TokenType::KEYWORD_INT:
        case TokenType::KEYWORD_CHAR:
        case TokenType::KEYWORD_VOID:
        case TokenType::KEYWORD_IF:
        case TokenType::KEYWORD_WHILE:
        case TokenType::KEYWORD_FOR:
        case TokenType::KEYWORD_RETURN:
            return;
        default:
            advance();
        }
    }
}

// ==================== Main Parse Method ====================

std::unique_ptr<ASTNode> Parser::parse()
{
    try
    {
        // Parse until we find a function declaration
        while (!isAtEnd())
        {
            auto decl = parseDeclaration();
            if (decl)
            {
                return decl; // Return the first function found
            }
        }
        return nullptr; // No function found
    }
    catch (...)
    {
        return nullptr;
    }
}

// ==================== Declaration Parsing ====================

std::unique_ptr<Decl> Parser::parseDeclaration()
{
    // Look for type keywords
    if (check(TokenType::KEYWORD_INT) ||
        check(TokenType::KEYWORD_VOID) ||
        check(TokenType::KEYWORD_CHAR))
    {

        // DON'T consume yet - parseFunctionDeclaration will do it
        return parseFunctionDeclaration();
    }

    // Skip unknown
    advance();
    return nullptr;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDeclaration()
{
    // Parse return type (already checked in parseDeclaration)
    Token typeToken = advance(); // Consume the type token
    std::string returnType = typeToken.lexeme;

    // Parse function name
    Token nameToken = consume(TokenType::IDENTIFIER, "Expect function name");
    std::string funcName = nameToken.lexeme;

    auto func = std::make_unique<FunctionDecl>(returnType, funcName, SourceLoc(nameToken));

    // Parse parameters
    consume(TokenType::PUNCT_LPAREN, "Expect '(' after function name");

    if (!match(TokenType::PUNCT_RPAREN))
    {
        // Skip parameters for Phase B
        while (!check(TokenType::PUNCT_RPAREN) && !isAtEnd())
        {
            advance();
            if (match(TokenType::PUNCT_COMMA))
                continue;
        }
        consume(TokenType::PUNCT_RPAREN, "Expect ')' after parameters");
    }

    // Parse function body - CONSUME '{' FIRST
    consume(TokenType::PUNCT_LBRACE, "Expect '{' before function body");
    func->body = parseCompoundStatement();

    return func;
}

std::unique_ptr<VarDecl> Parser::parseVariableDeclaration()
{
    // For Phase B: skip variable declarations
    // Implement in Phase C
    while (!match(TokenType::PUNCT_SEMICOLON) && !isAtEnd())
    {
        advance();
    }
    return nullptr;
}

// ==================== Statement Parsing ====================

std::unique_ptr<Stmt> Parser::parseStatement()
{
    if (match(TokenType::KEYWORD_RETURN))
        return parseReturnStatement();
    if (match(TokenType::KEYWORD_IF))
        return parseIfStatement();
    if (match(TokenType::KEYWORD_WHILE))
        return parseWhileStatement();
    if (match(TokenType::PUNCT_LBRACE))
        return parseCompoundStatement();

    return parseExpressionStatement();
}

std::unique_ptr<CompoundStmt> Parser::parseCompoundStatement()
{
    // std::cout << "DEBUG: parseCompoundStatement called at line " << peek().line << std::endl;

    auto compound = std::make_unique<CompoundStmt>(SourceLoc(previous()));

    while (!check(TokenType::PUNCT_RBRACE) && !isAtEnd())
    {
        auto stmt = parseStatement();
        if (stmt)
        {
            compound->statements.push_back(std::move(stmt));
        }
    }

    consume(TokenType::PUNCT_RBRACE, "Expect '}' after block");
    // std::cout << "DEBUG: parseCompoundStatement finished\n";
    return compound;
}

std::unique_ptr<IfStmt> Parser::parseIfStatement()
{
    SourceLoc loc(previous());

    consume(TokenType::PUNCT_LPAREN, "Expect '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::PUNCT_RPAREN, "Expect ')' after condition");

    auto thenBranch = parseStatement();
    std::unique_ptr<Stmt> elseBranch = nullptr;

    if (match(TokenType::KEYWORD_ELSE))
    {
        elseBranch = parseStatement();
    }

    return std::make_unique<IfStmt>(std::move(condition),
                                    std::move(thenBranch),
                                    std::move(elseBranch),
                                    loc);
}

std::unique_ptr<WhileStmt> Parser::parseWhileStatement()
{
    SourceLoc loc(previous());

    consume(TokenType::PUNCT_LPAREN, "Expect '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::PUNCT_RPAREN, "Expect ')' after condition");

    auto body = parseStatement();

    return std::make_unique<WhileStmt>(std::move(condition),
                                       std::move(body),
                                       loc);
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStatement()
{
    SourceLoc loc(previous());

    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::PUNCT_SEMICOLON))
    {
        value = parseExpression();
    }

    consume(TokenType::PUNCT_SEMICOLON, "Expect ';' after return value");

    return std::make_unique<ReturnStmt>(std::move(value), loc);
}

std::unique_ptr<ExprStmt> Parser::parseExpressionStatement()
{
    auto expr = parseExpression();
    auto stmt = std::make_unique<ExprStmt>(std::move(expr), SourceLoc(previous()));

    consume(TokenType::PUNCT_SEMICOLON, "Expect ';' after expression");

    return stmt;
}

// ==================== Expression Parsing ====================
// Using precedence climbing

std::unique_ptr<Expr> Parser::parseExpression()
{
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment()
{
    // For Phase B: no assignments, just equality
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality()
{
    auto expr = parseComparison();

    while (match(TokenType::OP_EQ) || match(TokenType::OP_NE))
    {
        Token op = previous();
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(op.type, std::move(expr), std::move(right),
                                            SourceLoc(op));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison()
{
    auto expr = parseTerm();

    while (match(TokenType::OP_LT) || match(TokenType::OP_LE) ||
           match(TokenType::OP_GT) || match(TokenType::OP_GE))
    {
        Token op = previous();
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(op.type, std::move(expr), std::move(right),
                                            SourceLoc(op));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm()
{
    auto expr = parseFactor();

    while (match(TokenType::OP_PLUS) || match(TokenType::OP_MINUS))
    {
        Token op = previous();
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(op.type, std::move(expr), std::move(right),
                                            SourceLoc(op));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor()
{
    auto expr = parseUnary();

    while (match(TokenType::OP_STAR) || match(TokenType::OP_SLASH) ||
           match(TokenType::OP_PERCENT))
    {
        Token op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(op.type, std::move(expr), std::move(right),
                                            SourceLoc(op));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary()
{
    if (match(TokenType::OP_MINUS) || match(TokenType::OP_LOGIC_NOT) ||
        match(TokenType::OP_BIT_NOT))
    {
        Token op = previous();
        auto operand = parseUnary();
        return std::make_unique<UnaryExpr>(op.type, std::move(operand), SourceLoc(op));
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary()
{
    if (match(TokenType::LITERAL_INT) || match(TokenType::LITERAL_FLOAT) ||
        match(TokenType::LITERAL_STRING) || match(TokenType::LITERAL_CHAR))
    {
        Token literal = previous();
        return std::make_unique<LiteralExpr>(literal.lexeme, literal.type, SourceLoc(literal));
    }

    if (match(TokenType::IDENTIFIER))
    {
        Token name = previous();

        // Check if it's a function call
        if (match(TokenType::PUNCT_LPAREN))
        {
            return finishCall(std::make_unique<VarExpr>(name.lexeme, SourceLoc(name)));
        }

        // Regular variable
        current--; // Go back to identifier
        advance(); // Advance again to consume it
        return std::make_unique<VarExpr>(name.lexeme, SourceLoc(name));
    }

    if (match(TokenType::PUNCT_LPAREN))
    {
        auto expr = parseExpression();
        consume(TokenType::PUNCT_RPAREN, "Expect ')' after expression");
        return expr;
    }

    error(peek(), "Expect expression");
    return nullptr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee)
{
    auto call = std::make_unique<CallExpr>(
        dynamic_cast<VarExpr *>(callee.get())->name,
        callee->location);

    if (!check(TokenType::PUNCT_RPAREN))
    {
        do
        {
            if (call->arguments.size() >= 255)
            {
                error(peek(), "Can't have more than 255 arguments");
            }
            call->arguments.push_back(parseExpression());
        } while (match(TokenType::PUNCT_COMMA));
    }

    consume(TokenType::PUNCT_RPAREN, "Expect ')' after arguments");

    return call;
}