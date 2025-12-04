// src/semantic/TypeChecker.hpp
#pragma once
#include <memory>
#include <string>
#include "SymbolTable.hpp"
#include "ast/ast_nodes.h"

class TypeChecker
{
private:
    SymbolTable *symbolTable;

    // Helper function
    std::string tokenTypeToOperatorString(TokenType tokenType);

public:
    TypeChecker(SymbolTable *symTable) : symbolTable(symTable) {}

    // Type compatibility checks
    bool areCompatible(DataType type1, DataType type2, const std::string &op = "");

    // Expression type inference
    DataType getExpressionType(std::shared_ptr<ASTNode> expr);

    // Type conversion
    DataType getResultType(DataType left, DataType right, const std::string &op);

    // Type from string
    DataType typeFromString(const std::string &typeStr);

    // Check function call
    bool checkFunctionCall(std::shared_ptr<CallExpr> call,
                           std::shared_ptr<Symbol> funcSymbol);

    // Utility functions
    bool isIntegerType(DataType type);
    bool isPointerType(DataType type);
    std::string dataTypeToString(DataType type);

    // Set symbol table (if needed)
    void setSymbolTable(SymbolTable *symTable) { symbolTable = symTable; }
};