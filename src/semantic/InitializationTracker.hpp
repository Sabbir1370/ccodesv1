// src/semantic/InitializationTracker.hpp
#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include "SymbolTable.hpp"
#include "ast/ast_nodes.h"

enum class InitState
{
    UNDEFINED, // Not initialized
    DEFINED,   // Definitely initialized
    MAYBE,     // Might be initialized (e.g., in one branch)
    ERROR      // Used before initialization
};

class InitializationTracker
{
private:
    SymbolTable *symbolTable;
    std::vector<std::string> errors;

    // Analysis methods
    void analyzeBlock(std::shared_ptr<ASTNode> block,
                      std::unordered_map<std::string, InitState> &varStates);

    void analyzeStatement(std::shared_ptr<ASTNode> stmt,
                          std::unordered_map<std::string, InitState> &varStates);

    void analyzeVarDeclaration(VarDecl *varDecl,
                               std::unordered_map<std::string, InitState> &varStates);

    void analyzeExpression(std::shared_ptr<Expr> expr,
                           std::unordered_map<std::string, InitState> &varStates,
                           bool isReadContext);

    void analyzeIfStatement(IfStmt *ifStmt,
                            std::unordered_map<std::string, InitState> &varStates);

    void analyzeWhileStatement(WhileStmt *whileStmt,
                               std::unordered_map<std::string, InitState> &varStates);

    void addError(const std::string &error) { errors.push_back(error); }

public:
    InitializationTracker(SymbolTable *symTable) : symbolTable(symTable) {}

    // Analyze a function for initialization issues
    void analyzeFunction(std::shared_ptr<FunctionDecl> func);

    // Get errors
    const std::vector<std::string> &getErrors() const { return errors; }

    // Clear state
    void clear() { errors.clear(); }

    // Set symbol table
    void setSymbolTable(SymbolTable *symTable) { symbolTable = symTable; }
};