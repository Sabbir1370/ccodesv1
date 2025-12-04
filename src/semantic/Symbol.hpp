#pragma once
#include <string>
#include <memory>
#include "ast/ast_nodes.h"
#include "utils/SourceLocation.hpp"

enum class SymbolType
{
    VARIABLE,
    FUNCTION,
    PARAMETER,
    TYPE
};

// enum class DataType
// {
//     INT,
//     CHAR,
//     VOID,
//     POINTER,
//     ARRAY,
//     UNKNOWN
// };

class Symbol
{
private:
    std::string name;
    SymbolType symType;
    DataType dataType;
    std::shared_ptr<ASTNode> declNode;
    SourceLocation location;
    bool isInitialized;
    bool isUsed;

public:
    Symbol(const std::string &name, SymbolType type, DataType dataType,
           const SourceLocation &loc, std::shared_ptr<ASTNode> node = nullptr);

    // Getters
    const std::string &getName() const { return name; }
    SymbolType getSymbolType() const { return symType; }
    DataType getDataType() const { return dataType; }
    const SourceLocation &getLocation() const { return location; }
    std::shared_ptr<ASTNode> getDeclNode() const { return declNode; }
    bool getIsInitialized() const { return isInitialized; }
    bool getIsUsed() const { return isUsed; }

    // Setters
    void setInitialized(bool val) { isInitialized = val; }
    void setUsed(bool val) { isUsed = val; }
    void setDataType(DataType type) { dataType = type; }

    std::string toString() const;
};