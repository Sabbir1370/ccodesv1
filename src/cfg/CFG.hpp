#pragma once
#include <vector>
#include <memory>
#include <string>
#include "semantic/Symbol.hpp"
#include "Edge.hpp" // Include Edge.hpp for EdgeType

class BasicBlock;
class FunctionDecl;

class CFG
{
private:
    std::shared_ptr<FunctionDecl> function;
    std::shared_ptr<BasicBlock> entryBlock;
    std::shared_ptr<BasicBlock> exitBlock;
    std::vector<std::shared_ptr<BasicBlock>> blocks;
    std::vector<std::shared_ptr<Edge>> edges;
    int nextBlockId;

public:
    CFG(std::shared_ptr<FunctionDecl> func);

    std::shared_ptr<BasicBlock> createBlock(const std::string &label = "");
    void addBlock(std::shared_ptr<BasicBlock> block);

    std::shared_ptr<Edge> createEdge(std::shared_ptr<BasicBlock> source,
                                     std::shared_ptr<BasicBlock> target,
                                     EdgeType type,
                                     const std::string &cond = "");
    void addEdge(std::shared_ptr<Edge> edge);

    std::shared_ptr<BasicBlock> getEntryBlock() const { return entryBlock; }
    std::shared_ptr<BasicBlock> getExitBlock() const { return exitBlock; }
    std::shared_ptr<FunctionDecl> getFunction() const { return function; }
    const std::vector<std::shared_ptr<BasicBlock>> &getBlocks() const { return blocks; }
    const std::vector<std::shared_ptr<Edge>> &getEdges() const { return edges; }

    void setEntryBlock(std::shared_ptr<BasicBlock> block);
    void setExitBlock(std::shared_ptr<BasicBlock> block);

    void print() const;
    bool verify() const;
    std::shared_ptr<BasicBlock> findBlockById(int id) const;
};