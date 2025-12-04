// src/semantic/InitializationTracker.cpp
#include "semantic/InitializationTracker.hpp"
#include "ast/ast_nodes.h"
#include <iostream>
#include <sstream>

// Helper functions to convert raw pointers to shared_ptr with no-op deleters
static std::shared_ptr<ASTNode> toSharedPtr(ASTNode *ptr)
{
    return std::shared_ptr<ASTNode>(ptr, [](auto *) {});
}

static std::shared_ptr<Expr> toSharedPtr(Expr *ptr)
{
    return std::shared_ptr<Expr>(ptr, [](auto *) {});
}

static std::shared_ptr<Stmt> toSharedPtr(Stmt *ptr)
{
    return std::shared_ptr<Stmt>(ptr, [](auto *) {});
}

static std::shared_ptr<Decl> toSharedPtr(Decl *ptr)
{
    return std::shared_ptr<Decl>(ptr, [](auto *) {});
}
// Helper function to convert InitState to string
static std::string initStateToString(InitState state)
{
    switch (state)
    {
    case InitState::UNDEFINED:
        return "UNDEFINED";
    case InitState::DEFINED:
        return "DEFINED";
    case InitState::MAYBE:
        return "MAYBE";
    case InitState::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

// Helper function to print variable states
static void printVarStates(const std::unordered_map<std::string, InitState> &varStates)
{
    for (const auto &[name, state] : varStates)
    {
        std::cout << "    " << name << ": " << initStateToString(state) << std::endl;
    }
}

void InitializationTracker::analyzeFunction(std::shared_ptr<FunctionDecl> func)
{
    if (!func)
    {
        addError("Null function provided to InitializationTracker");
        return;
    }

    std::cout << "[InitializationTracker] Analyzing function: "
              << func->getFunctionName() << std::endl;

    // Initialize variable states for all local variables
    std::unordered_map<std::string, InitState> varStates;

    // 1. Mark parameters as DEFINED (they're initialized by the caller)
    for (size_t i = 0; i < func->getParamCount(); ++i)
    {
        if (auto param = func->getParameter(i))
        {
            varStates[param->getVarName()] = InitState::DEFINED;
            std::cout << "  Parameter: " << param->getVarName()
                      << " marked as DEFINED" << std::endl;
        }
    }

    // 2. Mark local variables as UNDEFINED initially
    // TODO: This should traverse the function body to find all local variables
    // For now, we'll analyze as we encounter them

    // 3. Analyze the function body
    if (func->hasBody())
    {
        // func->getBody() returns CompoundStmt* (raw pointer)
        // Create a shared_ptr that doesn't delete the original
        std::shared_ptr<ASTNode> bodyShared(func->getBody(), [](ASTNode *) {});
        analyzeBlock(bodyShared, varStates);
    }

    // 4. Report final states
    std::cout << "  Final variable states:" << std::endl;
    for (const auto &[name, state] : varStates)
    {
        std::cout << "    " << name << ": " << initStateToString(state);
        if (state == InitState::UNDEFINED)
        {
            std::cout << " ⚠️  Variable may be uninitialized";
        }
        std::cout << std::endl;
    }

    std::cout << "[InitializationTracker] Analysis complete with "
              << errors.size() << " errors" << std::endl;
}

void InitializationTracker::analyzeBlock(std::shared_ptr<ASTNode> block,
                                         std::unordered_map<std::string, InitState> &varStates)
{
    if (!block)
    {
        return;
    }

    // Handle CompoundStmt specifically
    if (auto compoundStmt = std::dynamic_pointer_cast<CompoundStmt>(block))
    {
        std::cout << "  Analyzing compound statement with "
                  << compoundStmt->getStatementCount() << " statements" << std::endl;

        // Analyze each statement in order
        for (size_t i = 0; i < compoundStmt->getStatementCount(); ++i)
        {
            if (auto stmt = compoundStmt->getStatement(i))
            {
                analyzeStatement(toSharedPtr(stmt), varStates);
            }
        }
    }
}

void InitializationTracker::analyzeStatement(std::shared_ptr<ASTNode> stmt,
                                             std::unordered_map<std::string, InitState> &varStates)
{
    if (!stmt)
    {
        return;
    }

    // Handle different statement types

    // VarDecl statement
    if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(stmt))
    {
        analyzeVarDeclaration(varDecl.get(), varStates);
    }
    // ExprStmt
    else if (auto exprStmt = std::dynamic_pointer_cast<ExprStmt>(stmt))
    {
        if (auto expr = exprStmt->getExpression())
        {
            analyzeExpression(toSharedPtr(expr), varStates, false);
        }
    }
    // ReturnStmt
    else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt))
    {
        if (returnStmt->hasValue())
        {
            analyzeExpression(toSharedPtr(returnStmt->getValue()), varStates, true);
        }
    }
    // IfStmt
    else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt))
    {
        analyzeIfStatement(ifStmt.get(), varStates);
    }
    // WhileStmt
    else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt))
    {
        analyzeWhileStatement(whileStmt.get(), varStates);
    }
}

void InitializationTracker::analyzeVarDeclaration(VarDecl *varDecl,
                                                  std::unordered_map<std::string, InitState> &varStates)
{
    std::string varName = varDecl->getVarName();

    // Add variable to state map if not already present
    if (varStates.find(varName) == varStates.end())
    {
        varStates[varName] = InitState::UNDEFINED;
        std::cout << "    Declared variable: " << varName << " (UNDEFINED)" << std::endl;
    }

    // If there's an initializer, variable becomes DEFINED
    if (varDecl->hasInitializer())
    {
        varStates[varName] = InitState::DEFINED;
        std::cout << "    Variable " << varName << " initialized -> DEFINED" << std::endl;

        // Analyze the initializer expression
        analyzeExpression(toSharedPtr(varDecl->getInitializer()), varStates, true); // FIXED
    }
}

void InitializationTracker::analyzeExpression(std::shared_ptr<Expr> expr,
                                              std::unordered_map<std::string, InitState> &varStates,
                                              bool isReadContext)
{
    if (!expr)
    {
        return;
    }

    // Handle VarExpr: variable usage
    if (auto varExpr = dynamic_cast<VarExpr *>(expr.get()))
    {
        std::string varName = varExpr->getName();

        if (varName == "NULL")
        {
            return; // Skip NULL, it's always "initialized"
        }

        if (isReadContext)
        {
            // Variable is being read - check if it's initialized
            auto it = varStates.find(varName);
            if (it != varStates.end())
            {
                if (it->second == InitState::UNDEFINED)
                {
                    std::stringstream ss;
                    ss << "Use of possibly uninitialized variable: " << varName;
                    addError(ss.str());
                    // Mark as ERROR to avoid duplicate warnings
                    it->second = InitState::ERROR;
                }
            }
            else
            {
                // Variable not found in current scope - might be global or error
                std::cout << "    Variable " << varName << " not in local state map" << std::endl;
            }
        }
    }
    // Handle BinaryExpr: could be assignment
    else if (auto binaryExpr = dynamic_cast<BinaryExpr *>(expr.get()))
    {
        // TODO: Check if this is an assignment operator
        // For now, analyze both sides
        if (binaryExpr->getLeft())
        {
            analyzeExpression(toSharedPtr(binaryExpr->getLeft()), // FIXED
                              varStates, false);
        }
        if (binaryExpr->getRight())
        {
            analyzeExpression(toSharedPtr(binaryExpr->getRight()), // FIXED
                              varStates, true);
        }
    }
    // Handle CallExpr: function call
    else if (auto callExpr = dynamic_cast<CallExpr *>(expr.get()))
    {
        // Analyze all arguments
        for (size_t i = 0; i < callExpr->getArgCount(); ++i)
        {
            if (auto arg = callExpr->getArgument(i))
            {
                analyzeExpression(toSharedPtr(arg), varStates, true); // FIXED
            }
        }
    }
    // TODO: Handle other expression types
}

void InitializationTracker::analyzeIfStatement(IfStmt *ifStmt,
                                               std::unordered_map<std::string, InitState> &varStates)
{
    // std::cout << "    [SKIP] IfStmt analysis" << std::endl;
    // return;
    std::cout << "    Analyzing if statement" << std::endl;

    // Save current state
    auto beforeState = varStates;

    // Analyze condition (reads variables)
    if (ifStmt->getCondition())
    {
        // FIX: Use no-op deleter - AST owns the object
        std::shared_ptr<Expr> condPtr(ifStmt->getCondition(), [](Expr *) {});
        analyzeExpression(condPtr, varStates, true);
    }

    // Analyze then branch
    if (ifStmt->getThenBranch())
    {
        // Copy state for then branch
        auto thenState = varStates;

        // FIX: Use no-op deleter
        std::shared_ptr<Stmt> thenPtr(ifStmt->getThenBranch(), [](Stmt *) {});
        analyzeStatement(thenPtr, thenState);

        // Analyze else branch if present
        if (ifStmt->hasElseBranch() && ifStmt->getElseBranch())
        {
            auto elseState = varStates; // Start from before branch state

            // FIX: Use no-op deleter
            std::shared_ptr<Stmt> elsePtr(ifStmt->getElseBranch(), [](Stmt *) {});
            analyzeStatement(elsePtr, elseState);

            // Merge states: variable is DEFINED only if DEFINED in BOTH branches
            for (auto &[name, state] : varStates)
            {
                auto thenIt = thenState.find(name);
                auto elseIt = elseState.find(name);

                if (thenIt != thenState.end() && elseIt != elseState.end())
                {
                    if (thenIt->second == InitState::DEFINED &&
                        elseIt->second == InitState::DEFINED)
                    {
                        state = InitState::DEFINED;
                    }
                    else if (thenIt->second == InitState::DEFINED ||
                             elseIt->second == InitState::DEFINED)
                    {
                        state = InitState::MAYBE;
                    }
                }
            }
        }
        else
        {
            // No else branch, state remains as before
            varStates = beforeState;
        }
    }
}

void InitializationTracker::analyzeWhileStatement(WhileStmt *whileStmt,
                                                  std::unordered_map<std::string, InitState> &varStates)
{
    // std::cout << "    [SKIP] WhileStmt analysis" << std::endl;
    // return;
    std::cout << "    Analyzing while statement" << std::endl;

    // Analyze condition
    if (whileStmt->getCondition())
    {
        analyzeExpression(std::shared_ptr<Expr>(whileStmt->getCondition()), varStates, true);
    }

    // Analyze body
    if (whileStmt->getBody())
    {
        analyzeStatement(std::shared_ptr<Stmt>(whileStmt->getBody()), varStates);
    }

    // After loop, variables are MAYBE at best (might not have executed)
    for (auto &[name, state] : varStates)
    {
        if (state == InitState::DEFINED)
        {
            state = InitState::MAYBE;
        }
    }
}