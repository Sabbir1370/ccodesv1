// src/semantic/SemanticAnalyzer.hpp
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "ast/ast_nodes.h"
#include "ast/ast_visitor.h"
#include "SymbolTable.hpp"
#include "TypeChecker.hpp"
#include "InitializationTracker.hpp"

class SemanticError : public std::exception
{
private:
    std::string message;
    int line;
    int column;

public:
    SemanticError(const std::string &msg, int line = -1, int col = -1)
        : message(msg), line(line), column(col) {}

    const char *what() const noexcept override
    {
        return message.c_str();
    }

    int getLine() const { return line; }
    int getColumn() const { return column; }
};

class SemanticAnalyzer : public ASTVisitor
{
private:
    std::unique_ptr<SymbolTable> symbolTable;
    std::unique_ptr<TypeChecker> typeChecker;
    std::unique_ptr<InitializationTracker> initTracker;
    std::shared_ptr<ASTNode> astRoot;
    std::vector<SemanticError> errors;

    // State tracking
    bool inFunction;
    std::shared_ptr<Symbol> currentFunction;

public:
    SemanticAnalyzer();

    // Main analysis entry point
    void analyze(std::shared_ptr<ASTNode> root);

    // ===== PASS 1: Declaration Collection =====
    void collectDeclarations(std::shared_ptr<ASTNode> node);
    void processFunctionBody(CompoundStmt *body);

    // ===== PASS 2: Type Checking & Resolution =====
    void resolveAndTypeCheck(std::shared_ptr<ASTNode> node);

    // ===== PASS 3: Initialization Tracking =====
    void trackInitialization(std::shared_ptr<ASTNode> root);

    // ===== Visitor Overrides =====
    // MUST MATCH EXACTLY the method signatures in ast_visitor.h
    void visit(std::shared_ptr<FunctionDecl> node) override;
    void visit(std::shared_ptr<VarDecl> node) override;

    // REMOVED: Assignment node (not in your AST)
    // void visit(std::shared_ptr<Assignment> node) override;

    // UPDATED: Match your actual node names from ast_nodes.h
    void visit(std::shared_ptr<IfStmt> node) override;     // Changed from IfStatement
    void visit(std::shared_ptr<WhileStmt> node) override;  // Changed from WhileStatement
    void visit(std::shared_ptr<ReturnStmt> node) override; // Changed from ReturnStatement
    void visit(std::shared_ptr<BinaryExpr> node) override; // Changed from BinaryExpression
    void visit(std::shared_ptr<CallExpr> node) override;   // Changed from FunctionCall

    // ADDED: Optional handlers for other node types (use base implementation)
    void visit(std::shared_ptr<CompoundStmt> node) override { ASTVisitor::visit(node); }
    void visit(std::shared_ptr<ExprStmt> node) override { ASTVisitor::visit(node); }
    void visit(std::shared_ptr<UnaryExpr> node) override { ASTVisitor::visit(node); }
    void visit(std::shared_ptr<VarExpr> node) override { ASTVisitor::visit(node); }
    void visit(std::shared_ptr<LiteralExpr> node) override { ASTVisitor::visit(node); }

    // ===== Getters =====
    const std::vector<SemanticError> &getErrors() const { return errors; }
    SymbolTable *getSymbolTable() { return symbolTable.get(); }
    std::shared_ptr<ASTNode> getAST() const { return astRoot; }
    bool hasErrors() const { return !errors.empty(); }

private:
    void addError(const std::string &msg, int line = -1, int col = -1);
    DataType getTypeFromString(const std::string &typeStr);
    void processVariableDeclaration(std::shared_ptr<VarDecl> node);
    std::string tokenTypeToOperatorString(TokenType tokenType);
};