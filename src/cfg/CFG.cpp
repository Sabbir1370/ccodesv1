#include "CFG.hpp"
#include "BasicBlock.hpp"
#include <iostream>
#include <sstream>

CFG::CFG(std::shared_ptr<FunctionDecl> func)
    : function(func), entryBlock(nullptr), exitBlock(nullptr), nextBlockId(0) {}
std::shared_ptr<BasicBlock> CFG::createBlock(const std::string &label)
{
    auto block = std::make_shared<BasicBlock>(nextBlockId++, label);
    blocks.push_back(block);
    return block;
}

void CFG::addBlock(std::shared_ptr<BasicBlock> block)
{
    blocks.push_back(block);
}

std::shared_ptr<Edge> CFG::createEdge(std::shared_ptr<BasicBlock> source,
                                      std::shared_ptr<BasicBlock> target,
                                      EdgeType type,
                                      const std::string &cond)
{
    auto edge = std::make_shared<Edge>(source, target, type, cond);

    // Connect both directions
    source->addOutgoingEdge(edge);
    target->addIncomingEdge(edge);

    edges.push_back(edge);
    return edge;
}

void CFG::addEdge(std::shared_ptr<Edge> edge)
{
    edges.push_back(edge);
}

void CFG::setEntryBlock(std::shared_ptr<BasicBlock> block)
{
    entryBlock = block;
}

void CFG::setExitBlock(std::shared_ptr<BasicBlock> block)
{
    exitBlock = block;
}

void CFG::print() const
{
    if (!function)
    {
        std::cout << "CFG: No function associated\n";
        return;
    }

    std::cout << "\n=== CFG for function: " << function->getFunctionName() << " ===\n";

    for (const auto &block : blocks)
    {
        block->print(2);

        // Print outgoing edges
        for (const auto &edge : block->getOutgoingEdges())
        {
            std::cout << "    -> Block #" << edge->getTarget()->getId()
                      << " [" << edge->getTypeString() << "]";
            if (!edge->getCondition().empty())
            {
                std::cout << " (if " << edge->getCondition() << ")";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

bool CFG::verify() const
{
    if (!entryBlock)
    {
        std::cerr << "CFG Error: No entry block\n";
        return false;
    }

    for (const auto &block : blocks)
    {
        if (block->getOutgoingEdges().empty() && block != exitBlock)
        {
            std::cerr << "CFG Warning: Block #" << block->getId()
                      << " has no outgoing edges and is not exit block\n";
        }
    }

    return true;
}

std::shared_ptr<BasicBlock> CFG::findBlockById(int id) const
{
    for (const auto &block : blocks)
    {
        if (block->getId() == id)
            return block;
    }
    return nullptr;
}