// src/utils/FunctionExtractor.cpp - CLEANED UP
#include "FunctionExtractor.hpp"

std::vector<std::shared_ptr<FunctionDecl>>
FunctionExtractor::extractFunctions(std::shared_ptr<ASTNode> ast)
{
    std::vector<std::shared_ptr<FunctionDecl>> functions;

    if (!ast)
        return functions;

    // If root is a single function declaration
    if (auto funcDecl = dynamic_cast<FunctionDecl *>(ast.get()))
    {
        functions.push_back(std::shared_ptr<FunctionDecl>(funcDecl, [](FunctionDecl *) {}));
        return functions;
    }

    // If root is a compound statement (multiple top-level declarations)
    if (auto compound = dynamic_cast<CompoundStmt *>(ast.get()))
    {
        extractFromCompound(compound, functions);
    }

    return functions;
}

void FunctionExtractor::extractFromCompound(CompoundStmt *compound,
                                            std::vector<std::shared_ptr<FunctionDecl>> &functions)
{
    if (!compound)
        return;

    for (size_t i = 0; i < compound->getStatementCount(); i++)
    {
        Stmt *stmt = compound->getStatement(i);
        if (!stmt)
            continue;

        if (auto funcDecl = dynamic_cast<FunctionDecl *>(stmt))
        {
            functions.push_back(std::shared_ptr<FunctionDecl>(funcDecl, [](FunctionDecl *) {}));
        }
        else if (auto nestedCompound = dynamic_cast<CompoundStmt *>(stmt))
        {
            // Recursively search nested compound statements
            extractFromCompound(nestedCompound, functions);
        }
    }
}