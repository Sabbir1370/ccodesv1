#include "semantic/Symbol.hpp"
#include <sstream>

Symbol::Symbol(const std::string &name, SymbolType type, DataType dataType,
               const SourceLocation &loc, std::shared_ptr<ASTNode> node)
    : name(name), symType(type), dataType(dataType),
      declNode(node), location(loc), isInitialized(false), isUsed(false)
{
}

std::string Symbol::toString() const
{
    std::stringstream ss;
    ss << "Symbol: " << name << " [";

    switch (symType)
    {
    case SymbolType::VARIABLE:
        ss << "VAR";
        break;
    case SymbolType::FUNCTION:
        ss << "FUNC";
        break;
    case SymbolType::PARAMETER:
        ss << "PARAM";
        break;
    case SymbolType::TYPE:
        ss << "TYPE";
        break;
    default:
        ss << "UNKNOWN";
        break;
    }

    ss << ", Type: ";
    switch (dataType)
    {
    case DataType::INT:
        ss << "int";
        break;
    case DataType::CHAR:
        ss << "char";
        break;
    case DataType::VOID:
        ss << "void";
        break;
    case DataType::POINTER:
        ss << "pointer";
        break;
    case DataType::ARRAY:
        ss << "array";
        break;
    case DataType::UNKNOWN:
        ss << "unknown";
        break;
    }

    ss << ", Loc: " << location.getLine() << ":" << location.getColumn();
    ss << ", Init: " << (isInitialized ? "yes" : "no");
    ss << ", Used: " << (isUsed ? "yes" : "no");
    ss << "]";

    return ss.str();
}