#include "semantic/Scope.hpp"
#include <iostream>
#include <iomanip>

Scope::Scope(ScopeType type, const std::string &name, Scope *parent)
    : type(type), name(name), parent(parent)
{
}

bool Scope::insertSymbol(const std::string &name, std::shared_ptr<Symbol> symbol)
{
    // Check for redeclaration in current scope
    if (symbols.find(name) != symbols.end())
    {
        return false; // Symbol already exists in this scope
    }

    symbols[name] = symbol;
    return true;
}

std::shared_ptr<Symbol> Scope::lookup(const std::string &name) const
{
    // Check current scope first
    auto it = symbols.find(name);
    if (it != symbols.end())
    {
        return it->second;
    }

    // Recursively check parent scopes
    if (parent != nullptr)
    {
        return parent->lookup(name);
    }

    return nullptr; // Symbol not found
}

std::shared_ptr<Symbol> Scope::lookupInCurrentScope(const std::string &name) const
{
    auto it = symbols.find(name);
    if (it != symbols.end())
    {
        return it->second;
    }
    return nullptr;
}

void Scope::print(int depth) const
{
    std::string indent(depth * 2, ' ');

    std::cout << indent << "Scope: ";
    switch (type)
    {
    case ScopeType::GLOBAL:
        std::cout << "GLOBAL";
        break;
    case ScopeType::FUNCTION:
        std::cout << "FUNCTION '" << name << "'";
        break;
    case ScopeType::BLOCK:
        std::cout << "BLOCK";
        break;
    case ScopeType::LOOP:
        std::cout << "LOOP";
        break;
    case ScopeType::CONDITIONAL:
        std::cout << "CONDITIONAL";
        break;
    }
    std::cout << " (" << symbols.size() << " symbols)" << std::endl;

    // Print symbols in this scope
    for (const auto &[symName, symbol] : symbols)
    {
        std::cout << indent << "  " << symbol->toString() << std::endl;
    }
}