#include "parser.h"
#include <iostream>

// ==================== Helper Methods ====================

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens), current(0), hasError(false) {}

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

const Token &Parser::currentToken() const
{
    return tokens[current];
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
    // Return error token but don't get stuck
    return Token{TokenType::ERROR, "", peek().line, peek().column};
}

bool Parser::isTypeToken(TokenType type)
{
    return type == TokenType::KEYWORD_INT ||
           type == TokenType::KEYWORD_CHAR ||
           type == TokenType::KEYWORD_VOID ||
           type == TokenType::KEYWORD_DOUBLE ||
           type == TokenType::KEYWORD_FLOAT ||
           type == TokenType::KEYWORD_LONG ||
           type == TokenType::KEYWORD_SHORT ||
           type == TokenType::KEYWORD_SIGNED ||
           type == TokenType::KEYWORD_UNSIGNED;
}

void Parser::error(const std::string &message)
{
    std::cerr << "[Line " << peek().line << "] Error: " << message << std::endl;
    hasError = true;
}

void Parser::error(const Token &token, const std::string &message)
{
    std::cerr << "[Line " << token.line << "] Error: " << message << std::endl;
    hasError = true;
}

// FIXED: Proper synchronization without infinite loops
void Parser::synchronize()
{
    // Skip the current problematic token
    advance();

    // Skip tokens until we find a synchronization point
    while (!isAtEnd())
    {
        // If we just passed a semicolon or brace, we're at a boundary
        if (previous().type == TokenType::PUNCT_SEMICOLON ||
            previous().type == TokenType::PUNCT_RBRACE ||
            previous().type == TokenType::PUNCT_LBRACE)
            return;

        // Check if current token can start a new statement/declaration
        switch (peek().type)
        {
        // Type keywords that start declarations
        case TokenType::KEYWORD_INT:
        case TokenType::KEYWORD_CHAR:
        case TokenType::KEYWORD_VOID:
        case TokenType::KEYWORD_DOUBLE:
        case TokenType::KEYWORD_FLOAT:
        case TokenType::KEYWORD_LONG:
        case TokenType::KEYWORD_SHORT:
        case TokenType::KEYWORD_SIGNED:
        case TokenType::KEYWORD_UNSIGNED:
        // Control flow keywords
        case TokenType::KEYWORD_IF:
        case TokenType::KEYWORD_WHILE:
        case TokenType::KEYWORD_FOR:
        case TokenType::KEYWORD_DO:
        case TokenType::KEYWORD_SWITCH:
        case TokenType::KEYWORD_CASE:
        case TokenType::KEYWORD_DEFAULT:
        // Other statement starters
        case TokenType::KEYWORD_RETURN:
        case TokenType::KEYWORD_BREAK:
        case TokenType::KEYWORD_CONTINUE:
        case TokenType::KEYWORD_GOTO:
        // Braces and end markers
        case TokenType::PUNCT_LBRACE:
        case TokenType::PUNCT_RBRACE:
        case TokenType::END_OF_FILE:
            return; // Found synchronization point

        default:
            // Keep skipping tokens
            advance();
        }
    }
}

// ==================== NEW: Parameter Parsing ====================

// ==================== Parameter List Parsing (FIXED) ====================
void Parser::parseParameterList(std::vector<std::unique_ptr<VarDecl>> &params)
{
    if (check(TokenType::PUNCT_RPAREN))
    {
        return;
    }

    do
    {
        std::vector<std::string> typeParts;
        std::vector<std::string> qualifiers;

        // === PHASE 1: Collect type specifiers and qualifiers ===
        while (!isAtEnd())
        {
            TokenType type = peek().type;

            // Type keywords
            if (isTypeToken(type))
            {
                typeParts.push_back(advance().lexeme);
            }
            // Type qualifiers
            else if (type == TokenType::KEYWORD_CONST ||
                     type == TokenType::KEYWORD_VOLATILE)
            {
                qualifiers.push_back(advance().lexeme);
            }
            // Storage class (usually not in parameters, but handle it)
            else if (type == TokenType::KEYWORD_REGISTER ||
                     type == TokenType::KEYWORD_STATIC)
            {
                advance(); // Skip for now
            }
            // Struct/union/enum
            else if (type == TokenType::KEYWORD_STRUCT ||
                     type == TokenType::KEYWORD_UNION ||
                     type == TokenType::KEYWORD_ENUM)
            {
                typeParts.push_back(advance().lexeme);
                // Optional tag name
                if (peek().type == TokenType::IDENTIFIER)
                {
                    typeParts.push_back(advance().lexeme);
                }
            }
            // Identifier (size_t, custom types)
            else if (type == TokenType::IDENTIFIER)
            {
                typeParts.push_back(advance().lexeme);
                break; // Identifier usually ends type spec
            }
            // Pointer star
            else if (type == TokenType::OP_STAR)
            {
                break; // Handle in next phase
            }
            else
            {
                break; // Not part of type
            }
        }

        // Build type name
        std::string typeName;
        for (const auto &part : typeParts)
        {
            if (!typeName.empty())
                typeName += " ";
            typeName += part;
        }

        // === PHASE 2: Handle pointers ===
        while (match(TokenType::OP_STAR))
        {
            typeName += "*";

            // Qualifiers can come after star: char * const
            if (peek().type == TokenType::KEYWORD_CONST ||
                peek().type == TokenType::KEYWORD_VOLATILE)
            {
                typeName += " ";
                typeName += advance().lexeme;
            }
        }

        // Add qualifiers that came before type
        for (const auto &qual : qualifiers)
        {
            typeName = qual + " " + typeName;
        }

        // === PHASE 3: Parameter name ===
        std::string paramName;
        if (match(TokenType::IDENTIFIER))
        {
            paramName = previous().lexeme;
        }
        else
        {
            paramName = "_";
        }

        // === PHASE 4: Arrays ===
        if (match(TokenType::PUNCT_LBRACKET))
        {
            typeName += "[]";
            // Skip array size
            while (!isAtEnd() && !match(TokenType::PUNCT_RBRACKET))
            {
                advance();
            }
        }

        // Create parameter
        auto param = std::make_unique<VarDecl>(
            typeName.empty() ? "int" : typeName,
            paramName,
            nullptr,
            SourceLoc(previous()));

        params.push_back(std::move(param));

    } while (match(TokenType::PUNCT_COMMA));
}
// ==================== Main Parse Method ====================
std::unique_ptr<ASTNode> Parser::parse()
{
    // Create a compound statement to hold all top-level declarations
    auto compound = std::make_unique<CompoundStmt>();

    try
    {
        // Parse all declarations in the file
        while (!isAtEnd())
        {
            auto decl = parseDeclaration();
            if (decl)
            {
                // Add to compound statement
                compound->statements.push_back(std::unique_ptr<Stmt>(static_cast<Stmt *>(decl.release())));
            }
            else
            {
                // If parseDeclaration returns null, skip token
                advance();
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Parser exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown parser exception" << std::endl;
    }

    // Return the compound statement containing all functions
    // If empty, return nullptr
    if (compound->statements.empty())
    {
        return nullptr;
    }

    // If only one function, return it directly for compatibility
    if (compound->statements.size() == 1)
    {
        return std::move(compound->statements[0]);
    }

    return compound;
}
// ==================== Declaration Parsing ====================

std::unique_ptr<Decl> Parser::parseDeclaration() // CHANGE: Return Stmt
{
    // Look for type keywords OR identifier types (like size_t)
    if (isTypeToken(peek().type) || peek().type == TokenType::IDENTIFIER)
    {
        // Check if it's a function or variable declaration
        // Peek ahead to see if there's a '(' after the identifier

        size_t savePos = current;

        // Skip type and pointer stars
        advance(); // Skip type
        while (peek().type == TokenType::OP_STAR)
        {
            advance();
        }

        // Check what comes next
        if (peek().type == TokenType::IDENTIFIER)
        {
            advance(); // Skip identifier

            if (peek().type == TokenType::PUNCT_LPAREN)
            {
                // It's a function declaration
                current = savePos;
                return parseFunctionDeclaration(); // Returns unique_ptr<FunctionDecl>
            }
        }

        // Not a function, restore and try variable declaration
        current = savePos;
        return parseVariableDeclaration(); // Returns unique_ptr<VarDecl>
    }

    // Skip unknown tokens with synchronization
    if (!isAtEnd())
    {
        synchronize();
    }
    return nullptr;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDeclaration()
{
    // === STEP 1: Parse return type (could be pointer: "void *", "int *", etc.) ===
    std::string returnType;

    // Parse base type
    Token typeToken = advance();
    returnType = typeToken.lexeme;

    // Parse pointer stars after type
    while (match(TokenType::OP_STAR))
    {
        returnType += "*";
    }

    // === STEP 2: Parse function name ===
    Token nameToken = consume(TokenType::IDENTIFIER, "Expect function name");
    if (nameToken.type == TokenType::ERROR)
    {
        synchronize();
        return nullptr;
    }

    std::string funcName = nameToken.lexeme;

    // === STEP 3: Parse opening parenthesis ===
    if (!match(TokenType::PUNCT_LPAREN))
    {
        error("Expect '(' after function name");
        synchronize();
        return nullptr;
    }

    auto func = std::make_unique<FunctionDecl>(returnType, funcName, SourceLoc(nameToken));

    // === STEP 4: Parse parameters ===
    std::vector<std::unique_ptr<VarDecl>> params;
    parseParameterList(params);
    func->parameters = std::move(params);

    // === STEP 5: Parse closing parenthesis ===
    if (!match(TokenType::PUNCT_RPAREN))
    {
        error("Expect ')' after parameters");
        synchronize();
        return nullptr;
    }

    // === STEP 6: Parse function body ===
    if (!match(TokenType::PUNCT_LBRACE))
    {
        error("Expect '{' before function body");
        synchronize();
        return nullptr;
    }

    func->body = parseCompoundStatement();

    return func;
}

std::unique_ptr<VarDecl> Parser::parseVariableDeclaration()
{
    // Parse type (could be pointer: "void *", "int *", etc.)
    std::string typeName;

    // Parse base type
    Token typeToken = currentToken();

    // Check if it's a valid type (keyword or identifier like size_t)
    if (!isTypeToken(typeToken.type) && typeToken.type != TokenType::IDENTIFIER)
    {
        error("Expected type in variable declaration");
        synchronize();
        return nullptr;
    }

    advance(); // Consume type token
    typeName = typeToken.lexeme;

    // Parse pointer stars after type
    while (match(TokenType::OP_STAR))
    {
        typeName += "*";
    }

    Token nameToken = currentToken();
    if (nameToken.type != TokenType::IDENTIFIER)
    {
        error("Expected variable name");
        synchronize();
        return nullptr;
    }
    advance();

    // Check for array brackets
    bool isArray = false;
    if (match(TokenType::PUNCT_LBRACKET))
    {
        isArray = true;
        // Parse or skip array size
        if (currentToken().type == TokenType::LITERAL_INT)
        {
            advance();
        }
        if (!match(TokenType::PUNCT_RBRACKET))
        {
            error("Expected ']' after array size");
            synchronize();
            return nullptr;
        }
    }

    // Check for initializer
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::OP_ASSIGN))
    {
        // Try to parse string literal (common case)
        if (currentToken().type == TokenType::LITERAL_STRING)
        {
            Token literalToken = currentToken();
            initializer = std::make_unique<LiteralExpr>(
                literalToken.lexeme,
                literalToken.type,
                SourceLoc(literalToken));
            advance();
        }
        else
        {
            // Skip other initializers for now
            while (!isAtEnd() &&
                   currentToken().type != TokenType::PUNCT_SEMICOLON)
            {
                advance();
            }
        }
    }

    if (!match(TokenType::PUNCT_SEMICOLON))
    {
        error("Expected ';' after variable declaration");
        synchronize();
        return nullptr;
    }

    // For arrays, append [] to type name
    std::string fullTypeName = typeToken.lexeme;
    if (isArray)
    {
        fullTypeName += "[]";
    }

    return std::make_unique<VarDecl>(
        fullTypeName,
        nameToken.lexeme,
        std::move(initializer),
        SourceLoc(typeToken.line, typeToken.column));
}

// ==================== Statement Parsing ====================

std::unique_ptr<Stmt> Parser::parseStatement()
{
    // Check for variable declaration
    if (isTypeToken(peek().type))
    {
        auto varDecl = parseVariableDeclaration();
        if (varDecl)
        {
            return varDecl;
        }
        synchronize();
        return nullptr;
    }

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

// ==================== Return Statement ====================

std::unique_ptr<ReturnStmt> Parser::parseReturnStatement()
{
    SourceLoc loc(previous()); // 'return' token location

    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::PUNCT_SEMICOLON))
    {
        value = parseExpression();
    }

    consume(TokenType::PUNCT_SEMICOLON, "Expect ';' after return value");

    return std::make_unique<ReturnStmt>(std::move(value), loc);
}

// ==================== While Statement ====================

std::unique_ptr<WhileStmt> Parser::parseWhileStatement()
{
    SourceLoc loc(previous()); // 'while' token location

    consume(TokenType::PUNCT_LPAREN, "Expect '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::PUNCT_RPAREN, "Expect ')' after condition");

    auto body = parseStatement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), loc);
}

// ==================== Expression Statement ====================

std::unique_ptr<ExprStmt> Parser::parseExpressionStatement()
{
    auto expr = parseExpression();
    auto stmt = std::make_unique<ExprStmt>(std::move(expr), SourceLoc(previous()));

    consume(TokenType::PUNCT_SEMICOLON, "Expect ';' after expression");

    return stmt;
}

std::unique_ptr<CompoundStmt> Parser::parseCompoundStatement()
{
    auto compound = std::make_unique<CompoundStmt>(SourceLoc(previous()));

    while (!check(TokenType::PUNCT_RBRACE) && !isAtEnd())
    {
        auto stmt = parseStatement();
        if (stmt)
        {
            compound->statements.push_back(std::move(stmt));
        }
    }

    if (!match(TokenType::PUNCT_RBRACE))
    {
        error("Expect '}' after block");
        synchronize();
    }

    return compound;
}

// ... rest of parseStatement methods remain the same ...

std::unique_ptr<IfStmt> Parser::parseIfStatement()
{
    SourceLoc loc(previous());

    if (!match(TokenType::PUNCT_LPAREN))
    {
        error("Expect '(' after 'if'");
        synchronize();
        return nullptr;
    }

    auto condition = parseExpression();

    if (!match(TokenType::PUNCT_RPAREN))
    {
        error("Expect ')' after condition");
        synchronize();
        return nullptr;
    }

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

// ==================== Expression Parsing ====================
// Using precedence climbing
// ==================== Expression Precedence Chain ====================

std::unique_ptr<Expr> Parser::parseExpression()
{
    return parseAssignment();
}

// Assignment is right-associative: a = b = c
std::unique_ptr<Expr> Parser::parseAssignment()
{
    auto expr = parseConditional();

    // Check for assignment operators
    if (match(TokenType::OP_ASSIGN) ||
        match(TokenType::OP_PLUS_ASSIGN) ||
        match(TokenType::OP_MINUS_ASSIGN) ||
        match(TokenType::OP_TIMES_ASSIGN) ||
        match(TokenType::OP_DIV_ASSIGN) ||
        match(TokenType::OP_MOD_ASSIGN) ||
        match(TokenType::OP_AND_ASSIGN) ||
        match(TokenType::OP_OR_ASSIGN) ||
        match(TokenType::OP_XOR_ASSIGN) ||
        match(TokenType::OP_SHL_ASSIGN) ||
        match(TokenType::OP_SHR_ASSIGN))
    {
        Token op = previous();
        auto value = parseAssignment(); // Right-associative
        return std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(value), SourceLoc(op));
    }

    return expr;
}

// Conditional: a ? b : c
std::unique_ptr<Expr> Parser::parseConditional()
{
    auto expr = parseLogicalOr();

    if (match(TokenType::PUNCT_QUESTION))
    {
        Token question = previous();
        auto thenExpr = parseExpression();
        consume(TokenType::PUNCT_COLON, "Expect ':' in conditional expression");
        auto elseExpr = parseConditional();

        // You'd need a ConditionalExpr class
        // For now, return thenExpr as placeholder
        return std::move(thenExpr);
    }

    return expr;
}

// Logical OR: a || b
std::unique_ptr<Expr> Parser::parseLogicalOr()
{
    auto expr = parseLogicalAnd();

    while (match(TokenType::OP_LOGIC_OR))
    {
        Token op = previous();
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(right), SourceLoc(op));
    }

    return expr;
}

// Logical AND: a && b
std::unique_ptr<Expr> Parser::parseLogicalAnd()
{
    auto expr = parseBitwiseOr();

    while (match(TokenType::OP_LOGIC_AND))
    {
        Token op = previous();
        auto right = parseBitwiseOr();
        expr = std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(right), SourceLoc(op));
    }

    return expr;
}

// Bitwise OR: a | b
std::unique_ptr<Expr> Parser::parseBitwiseOr()
{
    auto expr = parseBitwiseXor();

    while (match(TokenType::OP_BIT_OR))
    {
        Token op = previous();
        auto right = parseBitwiseXor();
        expr = std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(right), SourceLoc(op));
    }

    return expr;
}

// Bitwise XOR: a ^ b
std::unique_ptr<Expr> Parser::parseBitwiseXor()
{
    auto expr = parseBitwiseAnd();

    while (match(TokenType::OP_BIT_XOR))
    {
        Token op = previous();
        auto right = parseBitwiseAnd();
        expr = std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(right), SourceLoc(op));
    }

    return expr;
}

// Bitwise AND: a & b
std::unique_ptr<Expr> Parser::parseBitwiseAnd()
{
    auto expr = parseEquality();

    while (match(TokenType::OP_BIT_AND))
    {
        Token op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpr>(
            op.type, std::move(expr), std::move(right), SourceLoc(op));
    }

    return expr;
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
    if (check(TokenType::IDENTIFIER) && peek().lexeme == "NULL")
    {
        Token nullToken = advance();
        // Create a literal 0 (NULL is (void*)0, but for our purposes, 0 works)
        return std::make_unique<LiteralExpr>("0", TokenType::LITERAL_INT, SourceLoc(nullToken));
    }
    // === 1. sizeof operator ===
    if (match(TokenType::KEYWORD_SIZEOF))
    {
        Token sizeofToken = previous();
        SourceLoc loc(sizeofToken);

        // Check for sizeof(type) or sizeof expr
        if (match(TokenType::PUNCT_LPAREN))
        {
            // Could be sizeof(type) or sizeof(expr)
            bool looksLikeType = false;
            size_t savePos = current;

            // Simple check: if next token is a type keyword, it's sizeof(type)
            if (isTypeToken(peek().type) ||
                peek().type == TokenType::KEYWORD_STRUCT ||
                peek().type == TokenType::KEYWORD_UNION ||
                peek().type == TokenType::KEYWORD_ENUM)
            {
                looksLikeType = true;
            }

            current = savePos; // Restore position

            if (looksLikeType)
            {
                // sizeof(type) - skip the type
                while (!isAtEnd() && !check(TokenType::PUNCT_RPAREN))
                {
                    advance();
                }
                if (!match(TokenType::PUNCT_RPAREN))
                {
                    error("Expected ')' after type in sizeof");
                    synchronize();
                }
            }
            else
            {
                // sizeof(expression)
                parseExpression(); // Parse but ignore for now
                if (!match(TokenType::PUNCT_RPAREN))
                {
                    error("Expected ')' after expression in sizeof");
                    synchronize();
                }
            }
        }
        else
        {
            // sizeof expression (without parentheses)
            parseExpression(); // Parse but ignore for now
        }

        // Create a placeholder expression for sizeof
        return std::make_unique<LiteralExpr>("0", TokenType::LITERAL_INT, loc);
    }

    // === 2. Unary increment/decrement (prefix) ===
    if (match(TokenType::OP_INCREMENT) || match(TokenType::OP_DECREMENT))
    {
        Token op = previous();
        auto operand = parsePrimary();
        return std::make_unique<UnaryExpr>(op.type, std::move(operand), SourceLoc(op));
    }

    // === 3. Cast expression or parenthesized expression ===
    if (match(TokenType::PUNCT_LPAREN))
    {
        // Check if it's a cast: (type)expr
        size_t savePos = current;
        bool isCast = false;

        // Try to parse as type
        if (isTypeToken(peek().type) || peek().type == TokenType::IDENTIFIER)
        {
            // Skip type name
            advance();
            // Skip pointer stars
            while (match(TokenType::OP_STAR))
            {
            }
            // Check for closing paren
            if (match(TokenType::PUNCT_RPAREN))
            {
                isCast = true;
            }
        }

        // Restore position
        current = savePos;

        if (isCast)
        {
            // Skip the cast type for now
            consume(TokenType::PUNCT_LPAREN, "Expect '('");
            while (!isAtEnd() && !match(TokenType::PUNCT_RPAREN))
            {
                advance();
            }
            // Parse the casted expression
            auto operand = parsePrimary();
            return operand;
        }
        else
        {
            // Parenthesized expression
            auto expr = parseExpression();
            consume(TokenType::PUNCT_RPAREN, "Expect ')' after expression");
            return expr;
        }
    }

    // === 4. Literals ===
    if (match(TokenType::LITERAL_INT) ||
        match(TokenType::LITERAL_FLOAT) ||
        match(TokenType::LITERAL_STRING) ||
        match(TokenType::LITERAL_CHAR))
    {
        Token literal = previous();
        return std::make_unique<LiteralExpr>(
            literal.lexeme, literal.type, SourceLoc(literal));
    }

    // === 5. Identifier (variable, function call, array, member access) ===
    if (match(TokenType::IDENTIFIER))
    {
        Token name = previous();
        std::unique_ptr<Expr> expr = std::make_unique<VarExpr>(name.lexeme, SourceLoc(name));

        // Postfix increment/decrement
        if (match(TokenType::OP_INCREMENT) || match(TokenType::OP_DECREMENT))
        {
            Token op = previous();
            // Create postfix expression
            return std::make_unique<UnaryExpr>(op.type, std::move(expr), SourceLoc(op));
        }

        // Check for function call
        if (match(TokenType::PUNCT_LPAREN))
        {
            return finishCall(std::move(expr));
        }

        // Parse postfix operations: array subscript, member access
        while (true)
        {
            // Array subscript: array[index]
            if (match(TokenType::PUNCT_LBRACKET))
            {
                auto index = parseExpression();
                consume(TokenType::PUNCT_RBRACKET, "Expect ']' after index");
                // Create array subscript node (use BinaryExpr as placeholder)
                expr = std::make_unique<BinaryExpr>(
                    TokenType::PUNCT_LBRACKET, // Use as operator placeholder
                    std::move(expr),
                    std::move(index),
                    SourceLoc(previous()));

                // Check for postfix inc/dec after array
                if (match(TokenType::OP_INCREMENT) || match(TokenType::OP_DECREMENT))
                {
                    Token op = previous();
                    expr = std::make_unique<UnaryExpr>(op.type, std::move(expr), SourceLoc(op));
                }
            }
            // Member access: struct.member
            else if (match(TokenType::PUNCT_DOT))
            {
                Token member = consume(TokenType::IDENTIFIER, "Expect member name after '.'");
                // Create member access (use BinaryExpr as placeholder)
                auto memberExpr = std::make_unique<VarExpr>(member.lexeme, SourceLoc(member));
                expr = std::make_unique<BinaryExpr>(
                    TokenType::PUNCT_DOT,
                    std::move(expr),
                    std::move(memberExpr),
                    SourceLoc(member));
            }
            // Pointer member access: ptr->member
            else if (match(TokenType::PUNCT_ARROW))
            {
                Token member = consume(TokenType::IDENTIFIER, "Expect member name after '->'");
                auto memberExpr = std::make_unique<VarExpr>(member.lexeme, SourceLoc(member));
                expr = std::make_unique<BinaryExpr>(
                    TokenType::PUNCT_ARROW,
                    std::move(expr),
                    std::move(memberExpr),
                    SourceLoc(member));
            }
            else
            {
                break;
            }
        }

        return expr;
    }

    // === 6. Error ===
    error(peek(), "Expect expression");
    synchronize();
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