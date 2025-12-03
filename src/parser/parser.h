#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "tokenizer.h"
#include "../ast/ast_nodes.h"

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);

    // Main parsing method
    std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;
    size_t current = 0;

    // Helper methods
    bool isAtEnd() const;
    const Token &peek() const;
    const Token &previous() const;
    const Token &advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &errorMessage);

    // Error handling
    void error(const Token &token, const std::string &message);
    void synchronize();

    // Parsing methods
    std::unique_ptr<Decl> parseDeclaration();
    std::unique_ptr<FunctionDecl> parseFunctionDeclaration();
    std::unique_ptr<VarDecl> parseVariableDeclaration();

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<CompoundStmt> parseCompoundStatement();
    std::unique_ptr<IfStmt> parseIfStatement();
    std::unique_ptr<WhileStmt> parseWhileStatement();
    std::unique_ptr<ReturnStmt> parseReturnStatement();
    std::unique_ptr<ExprStmt> parseExpressionStatement();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseFactor();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    // For function calls
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
};

#endif // PARSER_H