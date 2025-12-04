#pragma once
#include <vector>
#include <memory>
#include <stack>
#include <string>
#include "Scope.hpp"

class SymbolTable
{
private:
    std::unique_ptr<Scope> globalScope;
    std::stack<Scope *> scopeStack;

public:
    SymbolTable();
    ~SymbolTable(); // Consider adding if you need cleanup

    // Scope management
    void enterScope(ScopeType type, const std::string &name = "");
    void exitScope();
    Scope *getCurrentScope();
    Scope *getGlobalScope() { return globalScope.get(); }

    // Symbol operations
    bool insertSymbol(const std::string &name, std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> lookup(const std::string &name);
    std::shared_ptr<Symbol> lookupInCurrentScope(const std::string &name);

    // Optional helpers that might be useful:
    bool symbolExists(const std::string &name); // Wrapper around lookup
    size_t getScopeDepth() const { return scopeStack.size(); }
    bool isInGlobalScope() const { return scopeStack.size() == 1; }

    // Utility
    void print() const;

    // Error reporting
    std::string getCurrentScopeChain() const;
};