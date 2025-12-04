#pragma once
#include <memory>
#include <stack>
#include "semantic/SymbolTable.hpp"
#include "ast/ast_visitor.h"
#include "ast/ast_nodes.h" // Add this to get AST node types

// Forward declarations
class CFG;
class BasicBlock;
enum class EdgeType;

class CFGBuilder : public ASTVisitor
{
private:
    SymbolTable *symbolTable;
    std::shared_ptr<CFG> currentCFG;
    std::shared_ptr<BasicBlock> currentBlock;

    std::stack<std::shared_ptr<BasicBlock>> breakTargets;
    std::stack<std::shared_ptr<BasicBlock>> continueTargets;

    // Helper methods
    void startNewBlock(const std::string &label = "");
    void connectBlocks(std::shared_ptr<BasicBlock> from,
                       std::shared_ptr<BasicBlock> to,
                       EdgeType type,
                       const std::string &cond = "");

    void visitCompoundStmt(CompoundStmt *node); // Raw pointer
    void visitIfStmt(IfStmt *node);             // Raw pointer
    void visitWhileStmt(WhileStmt *node);       // Raw pointer
    void visitReturnStmt(ReturnStmt *node);     // Raw pointer
    void visitExprStmt(ExprStmt *node);         // Raw pointer
    void visitVarDecl(VarDecl *node);           // Raw pointer
public:
    CFGBuilder(SymbolTable *symTab);

    // Main entry point
    std::shared_ptr<CFG> buildCFG(std::shared_ptr<FunctionDecl> function);

    // ===== Visitor Interface (Minimal Implementation) =====
    virtual void visit(std::shared_ptr<FunctionDecl> node) override;
    virtual void visit(std::shared_ptr<CompoundStmt> node) override;
    virtual void visit(std::shared_ptr<IfStmt> node) override;
    virtual void visit(std::shared_ptr<WhileStmt> node) override;
    virtual void visit(std::shared_ptr<ReturnStmt> node) override;
    virtual void visit(std::shared_ptr<ExprStmt> node) override;
    virtual void visit(std::shared_ptr<VarDecl> node) override;

    virtual void visit(std::shared_ptr<BinaryExpr> node) override;
    virtual void visit(std::shared_ptr<UnaryExpr> node) override;
    virtual void visit(std::shared_ptr<CallExpr> node) override;
    virtual void visit(std::shared_ptr<VarExpr> node) override;
    virtual void visit(std::shared_ptr<LiteralExpr> node) override;

    virtual void visit(std::shared_ptr<ASTNode> node) override;
};