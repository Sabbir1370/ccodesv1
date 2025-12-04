#pragma once
#include <vector>
#include <memory>
#include "ast/ast_nodes.h"

class FunctionExtractor
{
public:
    static std::vector<std::shared_ptr<FunctionDecl>>
    extractFunctions(std::shared_ptr<ASTNode> ast);

private:
    static void extractFromCompound(CompoundStmt *compound, // CHANGE: Raw pointer
                                    std::vector<std::shared_ptr<FunctionDecl>> &functions);
};