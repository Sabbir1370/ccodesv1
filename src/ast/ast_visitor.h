// src/ast/ast_visitor.h
#pragma once
#include <memory>

// Forward declarations from ast_nodes.h
class ASTNode;
class FunctionDecl;
class VarDecl;
class CompoundStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class ExprStmt;
class BinaryExpr;
class UnaryExpr;
class CallExpr;
class VarExpr;
class LiteralExpr;

class ASTVisitor
{
public:
    virtual ~ASTVisitor() = default;

    // Visitor methods for different AST node types
    // These match the node types in your ast_nodes.h

    // Declarations
    virtual void visit(std::shared_ptr<FunctionDecl> node) {}
    virtual void visit(std::shared_ptr<VarDecl> node) {}

    // Statements
    virtual void visit(std::shared_ptr<CompoundStmt> node) {}
    virtual void visit(std::shared_ptr<IfStmt> node) {}
    virtual void visit(std::shared_ptr<WhileStmt> node) {}
    virtual void visit(std::shared_ptr<ReturnStmt> node) {}
    virtual void visit(std::shared_ptr<ExprStmt> node) {}

    // Expressions
    virtual void visit(std::shared_ptr<BinaryExpr> node) {}
    virtual void visit(std::shared_ptr<UnaryExpr> node) {}
    virtual void visit(std::shared_ptr<CallExpr> node) {}
    virtual void visit(std::shared_ptr<VarExpr> node) {}
    virtual void visit(std::shared_ptr<LiteralExpr> node) {}

    // Generic fallback for any ASTNode
    virtual void visit(std::shared_ptr<ASTNode> node) {}
};