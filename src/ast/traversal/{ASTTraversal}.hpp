#pragma once
#include <memory>
#include <vector>
#include "ast/ast_nodes.h"
#include "ast/ast_visitor.h"

class ASTTraversal
{
public:
    // Generic traversal methods
    static void traverse(std::shared_ptr<ASTNode> node, ASTVisitor &visitor);

    // Specific traversal patterns
    static void preOrderTraverse(std::shared_ptr<ASTNode> node, ASTVisitor &visitor);
    static void postOrderTraverse(std::shared_ptr<ASTNode> node, ASTVisitor &visitor);

    // Utility to find specific nodes
    static std::vector<std::shared_ptr<ASTNode>>
    findAllNodes(std::shared_ptr<ASTNode> root, ASTNodeType type);

    // Check if node is in a loop
    static bool isInLoopContext(std::shared_ptr<ASTNode> node);

    // Get parent function of a node
    static std::shared_ptr<FunctionDecl>
    getParentFunction(std::shared_ptr<ASTNode> node);
};