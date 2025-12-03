#include "ast_nodes.h"
#include <iostream>
#include <iomanip>

// ==================== Helper Functions ====================
static void printIndent(int indent)
{
    for (int i = 0; i < indent; ++i)
    {
        std::cout << "  ";
    }
}

// ==================== VarExpr ====================
void VarExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "VarExpr: " << name;
    if (!type.empty())
        std::cout << " (" << type << ")";
    std::cout << "\n";
}

// ==================== LiteralExpr ====================
void LiteralExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "LiteralExpr: " << value;
    if (!type.empty())
        std::cout << " (" << type << ")";
    std::cout << "\n";
}

// ==================== BinaryExpr ====================
void BinaryExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "BinaryExpr: op=" << static_cast<int>(op) << "\n";
    if (left)
    {
        printIndent(indent + 1);
        std::cout << "left:\n";
        left->print(indent + 2);
    }
    if (right)
    {
        printIndent(indent + 1);
        std::cout << "right:\n";
        right->print(indent + 2);
    }
}

// ==================== UnaryExpr ====================
void UnaryExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "UnaryExpr: op=" << static_cast<int>(op) << "\n";
    if (operand)
    {
        operand->print(indent + 1);
    }
}

// ==================== CallExpr ====================
void CallExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "CallExpr: " << funcName << "()\n";
    if (!arguments.empty())
    {
        printIndent(indent + 1);
        std::cout << "arguments:\n";
        for (const auto &arg : arguments)
        {
            arg->print(indent + 2);
        }
    }
}

// ==================== ReturnStmt ====================
void ReturnStmt::print(int indent) const
{
    printIndent(indent);
    std::cout << "ReturnStmt\n";
    if (value)
    {
        value->print(indent + 1);
    }
}

// ==================== CompoundStmt ====================
void CompoundStmt::print(int indent) const
{
    printIndent(indent);
    std::cout << "CompoundStmt {\n";
    for (const auto &stmt : statements)
    {
        stmt->print(indent + 1);
    }
    printIndent(indent);
    std::cout << "}\n";
}

// ==================== IfStmt ====================
void IfStmt::print(int indent) const
{
    printIndent(indent);
    std::cout << "IfStmt\n";

    printIndent(indent + 1);
    std::cout << "condition:\n";
    if (condition)
        condition->print(indent + 2);

    printIndent(indent + 1);
    std::cout << "then:\n";
    if (thenBranch)
        thenBranch->print(indent + 2);

    if (elseBranch)
    {
        printIndent(indent + 1);
        std::cout << "else:\n";
        elseBranch->print(indent + 2);
    }
}

// ==================== WhileStmt ====================
void WhileStmt::print(int indent) const
{
    printIndent(indent);
    std::cout << "WhileStmt\n";

    printIndent(indent + 1);
    std::cout << "condition:\n";
    if (condition)
        condition->print(indent + 2);

    printIndent(indent + 1);
    std::cout << "body:\n";
    if (body)
        body->print(indent + 2);
}

// ==================== ExprStmt ====================
void ExprStmt::print(int indent) const
{
    printIndent(indent);
    std::cout << "ExprStmt\n";
    if (expression)
        expression->print(indent + 1);
}

// ==================== VarDecl ====================
void VarDecl::print(int indent) const
{
    printIndent(indent);
    std::cout << "VarDecl: " << typeName << " " << varName;
    if (initializer)
    {
        std::cout << " = ";
        // Simple print for now
    }
    std::cout << "\n";
    if (initializer)
    {
        initializer->print(indent + 1);
    }
}

// ==================== FunctionDecl ====================
void FunctionDecl::print(int indent) const
{
    printIndent(indent);
    std::cout << "FunctionDecl: " << returnType << " " << funcName << "(";

    for (size_t i = 0; i < parameters.size(); ++i)
    {
        if (i > 0)
            std::cout << ", ";
        std::cout << parameters[i]->typeName << " " << parameters[i]->varName;
    }
    std::cout << ")\n";

    if (body)
    {
        body->print(indent + 1);
    }
}