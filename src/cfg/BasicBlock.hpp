#pragma once
#include <vector>
#include <memory>
#include <string>
#include "ast/ast_nodes.h"
#include "Edge.hpp"

class BasicBlock
{
private:
    int id;
    std::vector<std::shared_ptr<Stmt>> statements;
    std::vector<std::shared_ptr<Edge>> incomingEdges;
    std::vector<std::shared_ptr<Edge>> outgoingEdges;
    std::string label;

    // Helper to create shared_ptr from raw pointer with no-op deleter
    template <typename T>
    std::shared_ptr<T> makeSharedFromRaw(T *ptr)
    {
        return std::shared_ptr<T>(ptr, [](T *) { /* no-op deleter */ });
    }

public:
    BasicBlock(int blockId, const std::string &lbl = "");

    int getId() const { return id; }
    const std::vector<std::shared_ptr<Stmt>> &getStatements() const { return statements; }
    const std::vector<std::shared_ptr<Edge>> &getIncomingEdges() const { return incomingEdges; }
    const std::vector<std::shared_ptr<Edge>> &getOutgoingEdges() const { return outgoingEdges; }
    const std::string &getLabel() const { return label; }
    bool isEmpty() const { return statements.empty(); }

    // Accept raw pointers (matching your AST)
    void addStatement(Stmt *stmt);
    void addStatement(std::shared_ptr<Stmt> stmt); // Optional overload

    void addIncomingEdge(std::shared_ptr<Edge> edge);
    void addOutgoingEdge(std::shared_ptr<Edge> edge);

    void print(int indent = 0) const;
    std::string toString() const;

    std::shared_ptr<Stmt> getLastStatement() const;
    bool endsWithControlFlow() const;
};