#include "ast/ast_nodes.h"
#include "semantic/Symbol.hpp" // ADD THIS - to get Symbol class definition
#include <iostream>
#include <iomanip>
#include <sstream> // ADD THIS - for string operations

// ==================== TokenType to string helper ====================
// helper function (optional but useful for debugging)
std::string tokenTypeToString(TokenType type)
{
    // You should have this somewhere, or implement a simple version
    // Check if your tokenizer.h has string conversion
    return "TokenType[" + std::to_string(static_cast<int>(type)) + "]";
}

// Implementation of getDataTypeString
std::string ASTNode::getDataTypeString() const
{
    switch (nodeDataType)
    {
    case DataType::INT:
        return "int";
    case DataType::CHAR:
        return "char";
    case DataType::VOID:
        return "void";
    case DataType::POINTER:
        return "pointer";
    case DataType::ARRAY:
        return "array";
    case DataType::UNKNOWN:
        return "unknown";
    default:
        return "undefined";
    }
}

// Optional: Add inferDataType for LiteralExpr (in ast_nodes.cpp)
DataType LiteralExpr::inferDataType() const
{
    // Check if these TokenType values exist in your tokenizer.h
    // If not, you'll need to adjust based on your actual TokenType enum
    switch (literalType)
    {
    case TokenType::LITERAL_INT: // Make sure this exists
        return DataType::INT;
    case TokenType::LITERAL_FLOAT:
        // For simplicity, treat float as int for now
        return DataType::INT;
    case TokenType::LITERAL_CHAR:
        return DataType::CHAR;
    case TokenType::LITERAL_STRING:
        return DataType::POINTER; // char* in C
    default:
        return DataType::UNKNOWN;
    }
}

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

    // Show semantic info if available
    if (hasSymbol())
    {
        std::cout << " [symbol]";
    }
    if (hasDataType())
    {
        std::cout << " type:" << getDataTypeString();
    }
    else if (!type.empty())
    {
        std::cout << " (" << type << ")"; // legacy string type
    }
    std::cout << "\n";
}

// ==================== LiteralExpr ====================
void LiteralExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "LiteralExpr: " << value;

    // Show inferred type
    DataType inferred = inferDataType();
    if (inferred != DataType::UNKNOWN)
    {
        std::cout << " [";
        switch (inferred)
        {
        case DataType::INT:
            std::cout << "int";
            break;
        case DataType::CHAR:
            std::cout << "char";
            break;
        case DataType::POINTER:
            std::cout << "pointer";
            break;
        default:
            std::cout << "type:" << static_cast<int>(inferred);
        }
        std::cout << "]";
    }

    if (!type.empty())
        std::cout << " (" << type << ")";
    std::cout << "\n";
}

// ==================== BinaryExpr ====================
void BinaryExpr::print(int indent) const
{
    printIndent(indent);
    std::cout << "BinaryExpr: op=" << static_cast<int>(op);

    // Show result type if known
    if (hasDataType())
    {
        std::cout << " result:" << getDataTypeString();
    }
    std::cout << "\n";

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
    std::cout << "CallExpr: " << funcName << "()";

    // Show semantic info if available
    if (hasSymbol())
    {
        std::cout << " [has symbol]";
    }
    std::cout << "\n";

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

    // Show semantic info if available
    if (hasSymbol())
    {
        std::cout << " [symbol]";
    }
    if (hasDataType())
    {
        std::cout << " type:" << getDataTypeString();
    }

    if (initializer)
    {
        std::cout << " = ";
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

    // Print parameters
    for (size_t i = 0; i < parameters.size(); ++i)
    {
        if (i > 0)
            std::cout << ", ";
        std::cout << parameters[i]->typeName << " " << parameters[i]->varName;
    }
    std::cout << ")";

    // Show semantic info if available
    if (hasSymbol())
    {
        std::cout << " [symbol]";
    }
    std::cout << "\n";

    if (body)
    {
        body->print(indent + 1);
    }
}