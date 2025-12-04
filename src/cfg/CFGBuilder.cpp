#include "CFGBuilder.hpp"
#include "CFG.hpp"
#include "BasicBlock.hpp"
#include <iostream>
#include <memory>

CFGBuilder::CFGBuilder(SymbolTable *symTab)
    : symbolTable(symTab) {}

std::shared_ptr<CFG> CFGBuilder::buildCFG(std::shared_ptr<FunctionDecl> function)
{
    currentCFG = std::make_shared<CFG>(function);

    // Create entry block
    auto entryBlock = currentCFG->createBlock("entry");
    currentCFG->setEntryBlock(entryBlock);
    currentBlock = entryBlock;

    // Visit function body
    if (function->getBody())
    {
        visitCompoundStmt(function->getBody());
    }

    // Create exit block if not already created by return
    if (currentBlock && currentBlock->getOutgoingEdges().empty())
    {
        auto exitBlock = currentCFG->createBlock("exit");
        currentCFG->setExitBlock(exitBlock);
        currentCFG->createEdge(currentBlock, exitBlock, EdgeType::FALL_THROUGH);
    }

    return currentCFG;
}

void CFGBuilder::startNewBlock(const std::string &label)
{
    auto newBlock = currentCFG->createBlock(label);

    if (currentBlock)
    {
        currentCFG->createEdge(currentBlock, newBlock, EdgeType::FALL_THROUGH);
    }

    currentBlock = newBlock;
}

void CFGBuilder::connectBlocks(std::shared_ptr<BasicBlock> from,
                               std::shared_ptr<BasicBlock> to,
                               EdgeType type,
                               const std::string &cond)
{
    currentCFG->createEdge(from, to, type, cond);
}

// ===== Manual Traversal Methods (using raw pointers) =====

void CFGBuilder::visitCompoundStmt(CompoundStmt *node)
{
    if (!node)
        return;

    for (size_t i = 0; i < node->getStatementCount(); i++)
    {
        Stmt *stmt = node->getStatement(i);
        if (!stmt)
            continue;

        // Dispatch based on statement type
        if (auto ifStmt = dynamic_cast<IfStmt *>(stmt))
        {
            visitIfStmt(ifStmt);
        }
        else if (auto whileStmt = dynamic_cast<WhileStmt *>(stmt))
        {
            visitWhileStmt(whileStmt);
        }
        else if (auto returnStmt = dynamic_cast<ReturnStmt *>(stmt))
        {
            visitReturnStmt(returnStmt);
        }
        else if (auto exprStmt = dynamic_cast<ExprStmt *>(stmt))
        {
            visitExprStmt(exprStmt);
        }
        else if (auto varDecl = dynamic_cast<VarDecl *>(stmt))
        {
            visitVarDecl(varDecl);
        }
        else if (auto compoundStmt = dynamic_cast<CompoundStmt *>(stmt))
        {
            visitCompoundStmt(compoundStmt);
        }
        else
        {
            // Generic statement
            currentBlock->addStatement(stmt);
        }
    }
}

void CFGBuilder::visitIfStmt(IfStmt *node)
{
    if (!node)
        return;

    auto conditionBlock = currentBlock;

    // Add if statement to current block
    currentBlock->addStatement(node);

    // Create blocks for then and else
    auto thenBlock = currentCFG->createBlock("if_then");
    auto mergeBlock = currentCFG->createBlock("if_merge");

    // Connect condition to then (true branch)
    connectBlocks(conditionBlock, thenBlock, EdgeType::TRUE_BRANCH);

    // Process then branch
    currentBlock = thenBlock;
    if (node->getThenBranch())
    {
        if (auto thenCompound = dynamic_cast<CompoundStmt *>(node->getThenBranch()))
        {
            visitCompoundStmt(thenCompound);
        }
        else
        {
            currentBlock->addStatement(node->getThenBranch());
        }
    }
    connectBlocks(currentBlock, mergeBlock, EdgeType::FALL_THROUGH);

    // Handle else branch
    if (node->hasElseBranch())
    {
        auto elseBlock = currentCFG->createBlock("if_else");
        connectBlocks(conditionBlock, elseBlock, EdgeType::FALSE_BRANCH);

        currentBlock = elseBlock;
        if (node->getElseBranch())
        {
            if (auto elseCompound = dynamic_cast<CompoundStmt *>(node->getElseBranch()))
            {
                visitCompoundStmt(elseCompound);
            }
            else
            {
                currentBlock->addStatement(node->getElseBranch());
            }
        }
        connectBlocks(currentBlock, mergeBlock, EdgeType::FALL_THROUGH);
    }
    else
    {
        connectBlocks(conditionBlock, mergeBlock, EdgeType::FALSE_BRANCH);
    }

    currentBlock = mergeBlock;
}

void CFGBuilder::visitWhileStmt(WhileStmt *node)
{
    if (!node)
        return;

    auto loopHeader = currentCFG->createBlock("while_header");
    auto loopBody = currentCFG->createBlock("while_body");
    auto loopExit = currentCFG->createBlock("while_exit");

    connectBlocks(currentBlock, loopHeader, EdgeType::FALL_THROUGH);

    breakTargets.push(loopExit);
    continueTargets.push(loopHeader);

    currentBlock = loopHeader;
    currentBlock->addStatement(node);

    connectBlocks(currentBlock, loopBody, EdgeType::TRUE_BRANCH);
    connectBlocks(currentBlock, loopExit, EdgeType::FALSE_BRANCH);

    currentBlock = loopBody;
    if (node->getBody())
    {
        if (auto bodyCompound = dynamic_cast<CompoundStmt *>(node->getBody()))
        {
            visitCompoundStmt(bodyCompound);
        }
        else
        {
            currentBlock->addStatement(node->getBody());
        }
    }

    connectBlocks(currentBlock, loopHeader, EdgeType::LOOP_BACK);

    breakTargets.pop();
    continueTargets.pop();

    currentBlock = loopExit;
}

void CFGBuilder::visitReturnStmt(ReturnStmt *node)
{
    if (!node)
        return;

    currentBlock->addStatement(node);

    auto exitBlock = currentCFG->createBlock("exit");
    currentCFG->setExitBlock(exitBlock);
    connectBlocks(currentBlock, exitBlock, EdgeType::RETURN);

    currentBlock = exitBlock;
}

void CFGBuilder::visitExprStmt(ExprStmt *node)
{
    if (!node)
        return;
    currentBlock->addStatement(node);
}

void CFGBuilder::visitVarDecl(VarDecl *node)
{
    if (!node)
        return;
    currentBlock->addStatement(node);
}

// Visitor stubs (unchanged)
void CFGBuilder::visit(std::shared_ptr<FunctionDecl> node) {}

// ADD THESE:
void CFGBuilder::visit(std::shared_ptr<CompoundStmt> node) {}
void CFGBuilder::visit(std::shared_ptr<IfStmt> node) {}
void CFGBuilder::visit(std::shared_ptr<WhileStmt> node) {}
void CFGBuilder::visit(std::shared_ptr<ReturnStmt> node) {}
void CFGBuilder::visit(std::shared_ptr<ExprStmt> node) {}
void CFGBuilder::visit(std::shared_ptr<VarDecl> node) {}
void CFGBuilder::visit(std::shared_ptr<BinaryExpr> node) {}
void CFGBuilder::visit(std::shared_ptr<UnaryExpr> node) {}
void CFGBuilder::visit(std::shared_ptr<CallExpr> node) {}
void CFGBuilder::visit(std::shared_ptr<VarExpr> node) {}
void CFGBuilder::visit(std::shared_ptr<LiteralExpr> node) {}
void CFGBuilder::visit(std::shared_ptr<ASTNode> node) {}