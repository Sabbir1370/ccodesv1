#include "semantic/SymbolTable.hpp"
#include <iostream>
#include <stdexcept>

SymbolTable::SymbolTable()
{
    // Create global scope as the root
    globalScope = std::make_unique<Scope>(ScopeType::GLOBAL, "global");
    scopeStack.push(globalScope.get());
}
SymbolTable::~SymbolTable()
{
    // Clean up all dynamically allocated scopes (except global which is unique_ptr)
    while (!scopeStack.empty())
    {
        Scope *scope = scopeStack.top();
        scopeStack.pop();

        // Only delete if it's not the global scope (which is unique_ptr)
        if (scope != globalScope.get())
        {
            delete scope;
        }
    }
}
void SymbolTable::enterScope(ScopeType type, const std::string &name)
{
    Scope *current = getCurrentScope();
    if (!current)
    {
        throw std::runtime_error("No current scope when entering new scope");
    }

    Scope *newScope = new Scope(type, name, current);
    scopeStack.push(newScope);
}

void SymbolTable::exitScope()
{
    if (scopeStack.size() <= 1)
    {
        // Don't pop the global scope
        throw std::runtime_error("Attempted to exit global scope");
    }

    Scope *current = scopeStack.top(); // This variable is actually used now
    scopeStack.pop();

    // Delete the scope when we exit it
    delete current;
}

Scope *SymbolTable::getCurrentScope()
{
    if (scopeStack.empty())
    {
        return nullptr;
    }
    return scopeStack.top();
}

bool SymbolTable::insertSymbol(const std::string &name, std::shared_ptr<Symbol> symbol)
{
    Scope *current = getCurrentScope();
    if (!current)
    {
        return false;
    }

    // Check if symbol already exists in current scope
    if (current->lookupInCurrentScope(name))
    {
        return false; // Redeclaration error
    }

    return current->insertSymbol(name, symbol);
}

std::shared_ptr<Symbol> SymbolTable::lookup(const std::string &name)
{
    Scope *current = getCurrentScope();
    if (!current)
    {
        return nullptr;
    }

    return current->lookup(name);
}

std::shared_ptr<Symbol> SymbolTable::lookupInCurrentScope(const std::string &name)
{
    Scope *current = getCurrentScope();
    if (!current)
    {
        return nullptr;
    }

    return current->lookupInCurrentScope(name);
}

void SymbolTable::print() const
{
    std::cout << "=== Symbol Table Dump ===" << std::endl;
    std::cout << "Current scope chain: " << getCurrentScopeChain() << std::endl;
    std::cout << "Scope stack size: " << scopeStack.size() << std::endl;
    std::cout << std::endl;

    // Print from global scope down to current
    std::cout << "Global scope:" << std::endl;
    globalScope->print(1);

    // Print current scope if not global
    if (scopeStack.size() > 1)
    {
        std::cout << std::endl
                  << "Current scope:" << std::endl;
        scopeStack.top()->print(1);
    }

    std::cout << "=======================" << std::endl;
}

std::string SymbolTable::getCurrentScopeChain() const
{
    if (scopeStack.empty())
    {
        return "(empty)";
    }

    std::string chain;
    // We need to print from bottom (global) to top (current)
    // So we'll use a vector to reverse the stack
    std::vector<Scope *> scopes;

    // Copy stack to vector
    std::stack<Scope *> tempStack = scopeStack;
    while (!tempStack.empty())
    {
        scopes.push_back(tempStack.top());
        tempStack.pop();
    }

    // Print from global (last) to current (first)
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
    {
        Scope *scope = *it;

        if (!chain.empty())
        {
            chain += " -> ";
        }

        switch (scope->getType())
        {
        case ScopeType::GLOBAL:
            chain += "global";
            break;
        case ScopeType::FUNCTION:
            chain += "function:" + scope->getName();
            break;
        case ScopeType::BLOCK:
            chain += "block";
            break;
        case ScopeType::LOOP:
            chain += "loop";
            break;
        case ScopeType::CONDITIONAL:
            chain += "conditional";
            break;
        default:
            chain += "scope";
            break;
        }
    }

    return chain;
}