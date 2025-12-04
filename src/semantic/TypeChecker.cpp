// src/semantic/TypeChecker.cpp
#include "semantic/TypeChecker.hpp"
#include "ast/ast_nodes.h"
#include <iostream>
#include <sstream>

bool TypeChecker::areCompatible(DataType type1, DataType type2, const std::string &op)
{
    // Same types are always compatible
    if (type1 == type2)
    {
        return true;
    }

    // Handle UNKNOWN type (can be compatible with anything during analysis)
    if (type1 == DataType::UNKNOWN || type2 == DataType::UNKNOWN)
    {
        return true; // Allow during analysis, will be refined later
    }

    // int <-> char compatibility
    if ((type1 == DataType::INT && type2 == DataType::CHAR) ||
        (type1 == DataType::CHAR && type2 == DataType::INT))
    {
        // Most operations allow int/char mixing in C
        if (op.empty() || op == "=" || op == "+" || op == "-" ||
            op == "*" || op == "/" || op == "%" || op == "==" ||
            op == "!=" || op == "<" || op == ">" || op == "<=" ||
            op == ">=")
        {
            return true;
        }
    }

    // Pointer assignments (simplified - in real C, more complex)
    if (op == "=" && type1 == DataType::POINTER && type2 == DataType::POINTER)
    {
        return true; // TODO: Add proper pointer type checking
    }

    // NULL pointer assignment (0 to pointer)
    if (op == "=" && type1 == DataType::POINTER && type2 == DataType::INT)
    {
        return true; // 0 can be assigned to pointer (NULL)
    }

    // Array decay to pointer
    if (type1 == DataType::POINTER && type2 == DataType::ARRAY)
    {
        return true; // Arrays decay to pointers
    }

    return false;
}

DataType TypeChecker::getExpressionType(std::shared_ptr<ASTNode> expr)
{
    if (!expr)
    {
        return DataType::UNKNOWN;
    }

    // If node already has a resolved type, return it
    if (expr->hasDataType())
    {
        return expr->getDataType();
    }

    // Try to infer type based on node type
    // Use dynamic_cast to check node types
    if (auto varExpr = dynamic_cast<VarExpr *>(expr.get()))
    {
        // Variable expression - get type from symbol if available
        if (varExpr->hasSymbol())
        {
            return varExpr->getSymbol()->getDataType();
        }
        return DataType::UNKNOWN;
    }
    else if (auto literalExpr = dynamic_cast<LiteralExpr *>(expr.get()))
    {
        // Literal expression - infer from token type
        return literalExpr->inferDataType();
    }
    else if (auto binaryExpr = dynamic_cast<BinaryExpr *>(expr.get()))
    {
        // Binary expression - compute result type
        DataType leftType = getExpressionType(std::shared_ptr<Expr>(binaryExpr->getLeft()));
        DataType rightType = getExpressionType(std::shared_ptr<Expr>(binaryExpr->getRight()));

        // Convert TokenType to string op for getResultType
        std::string op = tokenTypeToOperatorString(binaryExpr->getOperator());
        return getResultType(leftType, rightType, op);
    }
    else if (auto callExpr = dynamic_cast<CallExpr *>(expr.get()))
    {
        // Function call - get return type from function symbol
        if (callExpr->hasSymbol())
        {
            return callExpr->getSymbol()->getDataType();
        }
        return DataType::UNKNOWN;
    }

    return DataType::UNKNOWN;
}

DataType TypeChecker::getResultType(DataType left, DataType right, const std::string &op)
{
    // Handle UNKNOWN types
    if (left == DataType::UNKNOWN)
        return right;
    if (right == DataType::UNKNOWN)
        return left;

    // Arithmetic operations
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%")
    {
        // Integer promotion rules (simplified)
        if (left == DataType::INT || right == DataType::INT)
        {
            return DataType::INT;
        }
        if (left == DataType::CHAR && right == DataType::CHAR)
        {
            return DataType::INT; // char + char promotes to int in C
        }
        return left; // Return the "dominant" type
    }

    // Comparison operations - always return int (0 or 1)
    if (op == "==" || op == "!=" || op == "<" || op == ">" ||
        op == "<=" || op == ">=")
    {
        return DataType::INT;
    }

    // Assignment - result is type of left operand
    if (op == "=")
    {
        return left;
    }

    // Logical operators - return int
    if (op == "&&" || op == "||")
    {
        return DataType::INT;
    }

    // Bitwise operators
    if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>")
    {
        if (left == DataType::INT || right == DataType::INT)
        {
            return DataType::INT;
        }
        return left;
    }

    return DataType::UNKNOWN;
}

DataType TypeChecker::typeFromString(const std::string &typeStr)
{
    if (typeStr == "int")
        return DataType::INT;
    if (typeStr == "char")
        return DataType::CHAR;
    if (typeStr == "void")
        return DataType::VOID;

    // Check for pointers
    if (typeStr.find('*') != std::string::npos)
    {
        return DataType::POINTER;
    }

    // Check for arrays
    if (typeStr.find('[') != std::string::npos)
    {
        return DataType::ARRAY;
    }

    // Handle const qualifier
    if (typeStr.find("const ") == 0)
    {
        std::string baseType = typeStr.substr(6);
        return typeFromString(baseType);
    }

    // Handle unsigned
    if (typeStr.find("unsigned ") == 0)
    {
        std::string baseType = typeStr.substr(9);
        return typeFromString(baseType);
    }

    return DataType::UNKNOWN;
}

bool TypeChecker::checkFunctionCall(std::shared_ptr<CallExpr> call,
                                    std::shared_ptr<Symbol> funcSymbol)
{
    if (!call || !funcSymbol)
    {
        return false;
    }

    // Basic check: function symbol should be a FUNCTION type
    if (funcSymbol->getSymbolType() != SymbolType::FUNCTION)
    {
        return false;
    }

    // TODO: Implement proper parameter checking
    // For Phase C, we'll do basic checks

    // Check argument count (simplified - real C has varargs, defaults, etc.)
    size_t argCount = call->getArgCount();

    // Log the check
    std::cout << "[TypeChecker] Checking function call: "
              << call->getFunctionName()
              << " with " << argCount << " arguments" << std::endl;

    // Check each argument type
    for (size_t i = 0; i < argCount; ++i)
    {
        if (auto arg = call->getArgument(i))
        {
            DataType argType = getExpressionType(std::shared_ptr<Expr>(arg));
            std::cout << "  Arg " << i << ": type=";
            switch (argType)
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
                std::cout << "unknown";
            }
            std::cout << std::endl;
        }
    }

    // For Phase C stub, return true if basic checks pass
    // We'll implement full parameter type matching in later phases
    return true;
}

// Helper function: Convert TokenType to operator string
std::string TypeChecker::tokenTypeToOperatorString(TokenType tokenType)
{
    // This is a simplified mapping - adjust based on your actual TokenType values
    switch (static_cast<int>(tokenType))
    {
    // Arithmetic
    case 1:
        return "+";
    case 2:
        return "-";
    case 3:
        return "*";
    case 4:
        return "/";
    case 5:
        return "%";

    // Comparison
    case 6:
        return "==";
    case 7:
        return "!=";
    case 8:
        return "<";
    case 9:
        return ">";
    case 10:
        return "<=";
    case 11:
        return ">=";

    // Assignment
    case 12:
        return "=";

    // Logical
    case 13:
        return "&&";
    case 14:
        return "||";

    // Bitwise
    case 15:
        return "&";
    case 16:
        return "|";
    case 17:
        return "^";
    case 18:
        return "<<";
    case 19:
        return ">>";

    default:
        return "";
    }
}

// Additional helper methods
bool TypeChecker::isIntegerType(DataType type)
{
    return type == DataType::INT || type == DataType::CHAR;
}

bool TypeChecker::isPointerType(DataType type)
{
    return type == DataType::POINTER || type == DataType::ARRAY;
}

std::string TypeChecker::dataTypeToString(DataType type)
{
    switch (type)
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