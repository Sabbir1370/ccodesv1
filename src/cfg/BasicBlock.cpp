#include "BasicBlock.hpp"
#include "Edge.hpp"
#include <iostream>

BasicBlock::BasicBlock(int blockId, const std::string &lbl)
    : id(blockId), label(lbl) {}

// Accept raw pointer
void BasicBlock::addStatement(Stmt *stmt)
{
    if (stmt)
    {
        statements.push_back(std::shared_ptr<Stmt>(stmt, [](Stmt *) {}));
    }
}

// Optional: Accept shared_ptr
void BasicBlock::addStatement(std::shared_ptr<Stmt> stmt)
{
    if (stmt)
    {
        statements.push_back(stmt);
    }
}

void BasicBlock::addIncomingEdge(std::shared_ptr<Edge> edge)
{
    incomingEdges.push_back(edge);
}

void BasicBlock::addOutgoingEdge(std::shared_ptr<Edge> edge)
{
    outgoingEdges.push_back(edge);
}

void BasicBlock::print(int indent) const
{
    std::string ind(indent, ' ');
    std::cout << ind << "Block #" << id;
    if (!label.empty())
        std::cout << " [" << label << "]";
    std::cout << " (" << statements.size() << " statements)\n";

    for (const auto &stmt : statements)
    {
        std::cout << ind << "  ";
        stmt->print(0);
    }
}

std::string BasicBlock::toString() const
{
    return "Block #" + std::to_string(id) +
           (label.empty() ? "" : " [" + label + "]") +
           " (" + std::to_string(statements.size()) + " statements)";
}

std::shared_ptr<Stmt> BasicBlock::getLastStatement() const
{
    if (statements.empty())
        return nullptr;
    return statements.back();
}

bool BasicBlock::endsWithControlFlow() const
{
    if (statements.empty())
        return false;

    auto lastStmt = statements.back();
    // Check if last statement is return, break, continue, or could branch
    // (Implementation depends on your AST node types)
    return false; // Placeholder
}