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

// Semantic system forward declarations
class Symbol;

// ==================== Data Type Enum ====================
enum class DataType
{
    INT,
    CHAR,
    VOID,
    POINTER,
    ARRAY,
    UNKNOWN
};

// ==================== Source Location ====================
struct SourceLoc
{
    int line = 0;
    int column = 0;

    SourceLoc() = default;
    SourceLoc(int l, int c) : line(l), column(c) {}
    SourceLoc(const Token &token) : line(token.line), column(token.column) {}

    // Helper to convert to semantic SourceLocation (if needed)
    bool isValid() const { return line > 0; }
    std::string toString() const
    {
        return "line " + std::to_string(line) + ", col " + std::to_string(column);
    }
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

    // ===== SEMANTIC ANALYSIS FIELDS =====
protected:
    std::shared_ptr<Symbol> resolvedSymbol;
    DataType nodeDataType = DataType::UNKNOWN;

public:
    // Symbol accessors
    void setSymbol(std::shared_ptr<Symbol> symbol) { resolvedSymbol = symbol; }
    std::shared_ptr<Symbol> getSymbol() const { return resolvedSymbol; }
    bool hasSymbol() const { return resolvedSymbol != nullptr; }

    // Type accessors
    void setDataType(DataType type) { nodeDataType = type; }
    DataType getDataType() const { return nodeDataType; }
    bool hasDataType() const { return nodeDataType != DataType::UNKNOWN; }

    // Utility
    std::string getDataTypeString() const;
};

// ==================== Expression Base Class ====================
class Expr : public ASTNode
{
public:
    // Type information (for semantic analysis later)
    std::string type; // "int", "float", "void", etc. (string representation)

    // Constructor
    Expr() = default;
};

// ==================== Statement Base Class ====================
class Stmt : public ASTNode
{
    // Base class for all statements
public:
    Stmt() = default;
};

// ==================== Declaration Base Class ====================
class Decl : public Stmt
{
    // Base class for declarations
public:
    Decl() = default;
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

    // Semantic helper
    const std::string &getName() const { return name; }
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

    // Semantic helper - infer DataType from literal type
    DataType inferDataType() const;
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

    // Semantic helper
    TokenType getOperator() const { return op; }
    Expr *getLeft() const { return left.get(); }
    Expr *getRight() const { return right.get(); }
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

    // Semantic helper
    TokenType getOperator() const { return op; }
    Expr *getOperand() const { return operand.get(); }
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

    // ===== SEMANTIC HELPER METHODS =====
    const std::string &getFunctionName() const { return funcName; }
    size_t getArgCount() const { return arguments.size(); }
    Expr *getArgument(size_t index) const
    {
        return (index < arguments.size()) ? arguments[index].get() : nullptr;
    }
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

    // Semantic helper
    bool hasValue() const { return value != nullptr; }
    Expr *getValue() const { return value.get(); }
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

    // Semantic helper
    size_t getStatementCount() const { return statements.size(); }
    Stmt *getStatement(size_t index) const
    {
        return (index < statements.size()) ? statements[index].get() : nullptr;
    }
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

    // Semantic helpers
    Expr *getCondition() const { return condition.get(); }
    Stmt *getThenBranch() const { return thenBranch.get(); }
    Stmt *getElseBranch() const { return elseBranch.get(); }
    bool hasElseBranch() const { return elseBranch != nullptr; }
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

    // Semantic helpers
    Expr *getCondition() const { return condition.get(); }
    Stmt *getBody() const { return body.get(); }
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

    // Semantic helper
    Expr *getExpression() const { return expression.get(); }
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

    // ===== SEMANTIC HELPER METHODS =====
    const std::string &getTypeName() const { return typeName; }
    const std::string &getVarName() const { return varName; }

    DataType getDeclaredDataType() const
    {
        // Convert typeName string to DataType enum
        if (typeName == "int")
            return DataType::INT;
        if (typeName == "char")
            return DataType::CHAR;
        if (typeName == "void")
            return DataType::VOID;
        if (typeName.find('*') != std::string::npos)
            return DataType::POINTER;
        if (typeName.find('[') != std::string::npos)
            return DataType::ARRAY;
        return DataType::UNKNOWN;
    }

    bool hasInitializer() const { return initializer != nullptr; }
    Expr *getInitializer() const { return initializer.get(); }
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

    // ===== SEMANTIC HELPER METHODS =====
    const std::string &getReturnType() const { return returnType; }
    const std::string &getFunctionName() const { return funcName; }

    DataType getReturnDataType() const
    {
        if (returnType == "int")
            return DataType::INT;
        if (returnType == "char")
            return DataType::CHAR;
        if (returnType == "void")
            return DataType::VOID;
        if (returnType.find('*') != std::string::npos)
            return DataType::POINTER;
        return DataType::UNKNOWN;
    }

    size_t getParamCount() const { return parameters.size(); }
    VarDecl *getParameter(size_t index) const
    {
        return (index < parameters.size()) ? parameters[index].get() : nullptr;
    }

    CompoundStmt *getBody() const { return body.get(); }
    bool hasBody() const { return body != nullptr; }
};

#endif // AST_NODES_H