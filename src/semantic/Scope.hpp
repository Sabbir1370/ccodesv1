#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "Symbol.hpp"

enum class ScopeType
{
    GLOBAL,
    FUNCTION,
    BLOCK,
    LOOP,
    CONDITIONAL
};

class Scope
{
private:
    ScopeType type;
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;
    Scope *parent;

public:
    Scope(ScopeType type, const std::string &name = "", Scope *parent = nullptr);

    bool insertSymbol(const std::string &name, std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> lookup(const std::string &name) const;
    std::shared_ptr<Symbol> lookupInCurrentScope(const std::string &name) const;

    Scope *getParent() const { return parent; }
    ScopeType getType() const { return type; }
    const std::string &getName() const { return name; }

    const auto &getSymbols() const { return symbols; }

    void print(int depth = 0) const;
};