#ifndef AST_NODES_H
#define AST_NODES_H

#include <memory>
#include <vector>
#include <string>
#include "parser/tokenizer.h" // For TokenType if needed

// ==================== Forward Declarations ====================
class ASTNode;
class Stmt;
class Expr;
class Decl;

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

// ==================== Source Location ====================
struct SourceLoc
{
    int line = 0;
    int column = 0;

    SourceLoc() = default;
    SourceLoc(int l, int c) : line(l), column(c) {}
    SourceLoc(const Token &token) : line(token.line), column(token.column) {}
};

// ==================== Base AST Node ====================
class ASTNode
{
public:
    virtual ~ASTNode() = default;

    // For debugging
    virtual void print(int indent = 0) const = 0;

    // Source location for error reporting
    SourceLoc location;
};

// ==================== Expression Base Class ====================
class Expr : public ASTNode
{
public:
    // Type information (for semantic analysis later)
    std::string type; // "int", "float", "void", etc.
};

// ==================== Statement Base Class ====================
class Stmt : public ASTNode
{
    // Base class for all statements
};

// ==================== Declaration Base Class ====================
class Decl : public ASTNode
{
    // Base class for declarations
};

// ==================== Specific Expression Nodes ====================

// Variable expression: "x"
class VarExpr : public Expr
{
public:
    std::string name;

    VarExpr(const std::string &n, SourceLoc loc = {}) : name(n)
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Literal expression: 42, 3.14, "hello"
class LiteralExpr : public Expr
{
public:
    std::string value;
    TokenType literalType; // LITERAL_INT, LITERAL_FLOAT, LITERAL_STRING, etc.

    LiteralExpr(const std::string &val, TokenType type, SourceLoc loc = {})
        : value(val), literalType(type)
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Binary expression: a + b, x == y
class BinaryExpr : public Expr
{
public:
    TokenType op; // OP_PLUS, OP_EQ, etc.
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(TokenType o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r,
               SourceLoc loc = {})
        : op(o), left(std::move(l)), right(std::move(r))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Unary expression: -x, !flag
class UnaryExpr : public Expr
{
public:
    TokenType op; // OP_MINUS, OP_LOGIC_NOT, etc.
    std::unique_ptr<Expr> operand;

    UnaryExpr(TokenType o, std::unique_ptr<Expr> expr, SourceLoc loc = {})
        : op(o), operand(std::move(expr))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Function call: func(arg1, arg2)
class CallExpr : public Expr
{
public:
    std::string funcName;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(const std::string &name, SourceLoc loc = {}) : funcName(name)
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// ==================== Specific Statement Nodes ====================

// Return statement: return expr;
class ReturnStmt : public Stmt
{
public:
    std::unique_ptr<Expr> value;

    ReturnStmt(std::unique_ptr<Expr> val = nullptr, SourceLoc loc = {})
        : value(std::move(val))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Compound statement: { stmt1; stmt2; ... }
class CompoundStmt : public Stmt
{
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    CompoundStmt(SourceLoc loc = {})
    {
        location = loc;
    }
    void print(int indent = 0) const override;
};

// If statement: if (cond) thenStmt else elseStmt
class IfStmt : public Stmt
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // can be null

    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenStmt,
           std::unique_ptr<Stmt> elseStmt = nullptr, SourceLoc loc = {})
        : condition(std::move(cond)),
          thenBranch(std::move(thenStmt)),
          elseBranch(std::move(elseStmt))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// While statement: while (cond) body
class WhileStmt : public Stmt
{
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b,
              SourceLoc loc = {})
        : condition(std::move(cond)), body(std::move(b))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Expression statement: expr; (function calls, assignments)
class ExprStmt : public Stmt
{
public:
    std::unique_ptr<Expr> expression;

    ExprStmt(std::unique_ptr<Expr> expr, SourceLoc loc = {})
        : expression(std::move(expr))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// ==================== Specific Declaration Nodes ====================

// Variable declaration: int x = 5;
class VarDecl : public Decl
{
public:
    std::string typeName; // "int", "float", etc.
    std::string varName;
    std::unique_ptr<Expr> initializer; // can be null

    VarDecl(const std::string &type, const std::string &name,
            std::unique_ptr<Expr> init = nullptr, SourceLoc loc = {})
        : typeName(type), varName(name), initializer(std::move(init))
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

// Function declaration: int main() { ... }
class FunctionDecl : public Decl
{
public:
    std::string returnType; // "int", "void", etc.
    std::string funcName;
    std::vector<std::unique_ptr<VarDecl>> parameters;
    std::unique_ptr<CompoundStmt> body;

    FunctionDecl(const std::string &retType, const std::string &name,
                 SourceLoc loc = {})
        : returnType(retType), funcName(name)
    {
        location = loc;
    }

    void print(int indent = 0) const override;
};

#endif // AST_NODES_H