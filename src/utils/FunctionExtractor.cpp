#include "FunctionExtractor.hpp"
#include <iostream>

std::vector<std::shared_ptr<FunctionDecl>>
FunctionExtractor::extractFunctions(std::shared_ptr<ASTNode> ast)
{
    std::vector<std::shared_ptr<FunctionDecl>> functions;

    if (!ast)
        return functions;

    // DEBUG: Print AST type
    std::cout << "  [DEBUG] AST type: ";
    if (dynamic_cast<FunctionDecl *>(ast.get()))
        std::cout << "FunctionDecl";
    else if (dynamic_cast<CompoundStmt *>(ast.get()))
        std::cout << "CompoundStmt";
    else
        std::cout << "Other";
    std::cout << std::endl;

    // If root is a single function declaration
    if (auto funcDecl = dynamic_cast<FunctionDecl *>(ast.get()))
    {
        functions.push_back(std::shared_ptr<FunctionDecl>(funcDecl, [](FunctionDecl *) {}));
        std::cout << "  [DEBUG] Found single function: " << funcDecl->getFunctionName() << std::endl;
        return functions;
    }

    // If root is a compound statement (multiple top-level declarations)
    if (auto compound = dynamic_cast<CompoundStmt *>(ast.get()))
    {
        std::cout << "  [DEBUG] Searching compound statement with "
                  << compound->getStatementCount() << " statements" << std::endl;
        extractFromCompound(compound, functions);
    }

    std::cout << "  [DEBUG] Total functions found: " << functions.size() << std::endl;
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

        // DEBUG
        std::cout << "  [DEBUG] Statement " << i << " type: ";

        if (auto funcDecl = dynamic_cast<FunctionDecl *>(stmt))
        {
            std::cout << "FunctionDecl: " << funcDecl->getFunctionName();
            functions.push_back(std::shared_ptr<FunctionDecl>(funcDecl, [](FunctionDecl *) {}));
        }
        else if (dynamic_cast<CompoundStmt *>(stmt))
        {
            std::cout << "CompoundStmt";
            // Recursively search nested compound statements
            extractFromCompound(dynamic_cast<CompoundStmt *>(stmt), functions);
        }
        else if (dynamic_cast<VarDecl *>(stmt))
        {
            std::cout << "VarDecl";
        }
        else if (dynamic_cast<IfStmt *>(stmt))
        {
            std::cout << "IfStmt";
        }
        else if (dynamic_cast<WhileStmt *>(stmt))
        {
            std::cout << "WhileStmt";
        }
        else if (dynamic_cast<ExprStmt *>(stmt))
        {
            std::cout << "ExprStmt";
        }
        else
        {
            std::cout << "Other";
        }
        std::cout << std::endl;
    }
}