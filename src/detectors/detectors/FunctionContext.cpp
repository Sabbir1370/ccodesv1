// src/detectors/detectors/FunctionContext.cpp - UPDATED
#include "FunctionContext.hpp"
#include "semantic/SymbolTable.hpp"
#include "semantic/Symbol.hpp"
#include <queue>
#include <algorithm>
#include <iostream>
#include <sstream>
#define TAINT_DEBUG_ENABLED 0
using namespace detectors;

FunctionContext::FunctionContext(std::shared_ptr<CFG> cfg)
    : cfg_(cfg)
{
    if (cfg && cfg->getFunction())
    {
        function_name_ = cfg->getFunction()->getFunctionName();
    }
}

void FunctionContext::initializeWorklist()
{
    // Start with the entry block
    auto entry = cfg_->getEntryBlock();
    if (!entry)
        return;

    worklist_.clear();
    worklist_.push_back(entry);

    // DO NOT reset taint state here! It should accumulate.
    // Only initialize if not already present
    for (const auto &block : cfg_->getBlocks())
    {
        int block_id = block->getId();
        if (block_taint_state_.find(block_id) == block_taint_state_.end())
        {
            block_taint_state_[block_id] = {}; // Only if not exists
        }
    }
}
void FunctionContext::addTaint(const std::string &identifier,
                               TaintSourceType source_type,
                               const SourceLocation &location,
                               std::shared_ptr<Expr> expr)
{

    TaintedValue taint;
    taint.identifier = identifier;
    taint.source_type = source_type;
    taint.source_location = location;
    taint.expr_node = expr;
    taint.taint_depth = 0;

    // Add to all blocks' taint sets that haven't been processed yet
    for (auto &pair : block_taint_state_)
    {
        pair.second.insert(taint);
    }
}

bool FunctionContext::isTaintSourceCall(CallExpr *call)
{
    if (!call)
        return false;

    std::string func_name = call->funcName;

    // Check against known taint sources
    if (taint_sources_.find(func_name) != taint_sources_.end())
    {
        return true;
    }

    // Also check for variants
    if (func_name.find("scanf") != std::string::npos ||
        func_name.find("gets") != std::string::npos ||
        func_name.find("read") != std::string::npos)
    {
        return true;
    }

    return false;
}

bool FunctionContext::isTaintSinkCall(CallExpr *call)
{
    if (!call)
        return false;

    std::string func_name = call->funcName;

    // Check against known taint sinks
    if (taint_sinks_.find(func_name) != taint_sinks_.end())
    {
        return true;
    }

    // Check for dangerous patterns
    if (func_name.find("strcpy") != std::string::npos ||
        func_name.find("system") != std::string::npos ||
        func_name.find("exec") != std::string::npos)
    {
        return true;
    }

    return false;
}

void FunctionContext::propagateTaintThroughAssignment(
    Expr *lhs,
    Expr *rhs,
    std::unordered_set<TaintedValue, TaintedValueHash> &current_taint)
{

    if (!lhs || !rhs)
        return;

    // Check if RHS is tainted
    std::vector<TaintedValue> rhs_taints;

    // For VarExpr on RHS
    if (auto var_expr = dynamic_cast<VarExpr *>(rhs))
    {
        std::string var_name = var_expr->name;

        // Find if this variable is tainted
        for (const auto &taint : current_taint)
        {
            if (taint.identifier == var_name)
            {
                rhs_taints.push_back(taint);
            }
        }
    }
    // For CallExpr on RHS (function return)
    else if (auto call_expr = dynamic_cast<CallExpr *>(rhs))
    {
        if (isTaintSourceCall(call_expr))
        {
            TaintedValue taint;
            taint.identifier = "ret_" + call_expr->funcName;
            taint.source_type = TaintSourceType::USER_INPUT;
            taint.source_location = convertSourceLoc(call_expr->location);
            // Note: We can't store raw pointer in shared_ptr without ownership
            // taint.expr_node = std::shared_ptr<Expr>(call_expr, [](Expr*){});
            taint.taint_depth = 0;
            rhs_taints.push_back(taint);
        }
    }

    // If RHS has taints, propagate to LHS
    if (!rhs_taints.empty())
    {
        if (auto lhs_var = dynamic_cast<VarExpr *>(lhs))
        {
            std::string lhs_name = lhs_var->name;

            for (const auto &rhs_taint : rhs_taints)
            {
                TaintedValue new_taint = rhs_taint;
                new_taint.identifier = lhs_name;
                new_taint.taint_depth = rhs_taint.taint_depth + 1;
                current_taint.insert(new_taint);
            }
        }
    }
}
void FunctionContext::analyzeExpression(
    Expr *expr,
    std::unordered_set<TaintedValue, TaintedValueHash> &current_taint)
{

    if (!expr)
        return;

    // Handle BinaryExpr for assignments (x = y)
    if (auto binary_expr = dynamic_cast<BinaryExpr *>(expr))
    {
        // TODO: Check if this is an assignment operator
        // For now, assume it's assignment and propagate taint
        propagateTaintThroughAssignment(binary_expr->left.get(),
                                        binary_expr->right.get(),
                                        current_taint);
    }
    // Keep existing CallExpr handling
    else if (auto call_expr = dynamic_cast<CallExpr *>(expr))
    {
        // Your existing code for sink checking
        if (isTaintSinkCall(call_expr))
        {
            // Check arguments for taint
            for (size_t i = 0; i < call_expr->arguments.size(); i++)
            {
                Expr *arg = call_expr->arguments[i].get();
                if (auto var_arg = dynamic_cast<VarExpr *>(arg))
                {
                    std::string arg_name = var_arg->name;

                    for (const auto &taint : current_taint)
                    {
                        if (taint.identifier == arg_name)
                        {
                            // Found tainted argument in sink
                        }
                    }
                }
            }
        }
    }
}
void FunctionContext::analyzeStatement(
    Stmt *stmt,
    std::unordered_set<TaintedValue, TaintedValueHash> &current_taint)
{

    if (!stmt)
        return;

    // Handle Expression Statement
    if (auto expr_stmt = dynamic_cast<ExprStmt *>(stmt))
    {
        Expr *expr = expr_stmt->expression.get();

        // DIRECTLY check for CallExpr that might be taint source/sink
        if (auto call_expr = dynamic_cast<CallExpr *>(expr))
        {
            // Check if this is a taint SOURCE (scanf, gets, etc.)
            if (isTaintSourceCall(call_expr))
            {
                std::cout << "[TAINT] Found taint source: " << call_expr->funcName << std::endl;

                // For scanf("%s", buffer) - taint the buffer argument
                // scanf arguments: format string, buffer
                if (call_expr->arguments.size() >= 2)
                {
                    Expr *buffer_arg = call_expr->arguments[1].get(); // Second arg is buffer

                    if (auto var_expr = dynamic_cast<VarExpr *>(buffer_arg))
                    {
                        std::string var_name = var_expr->name;

                        TaintedValue taint;
                        taint.identifier = var_name;
                        taint.source_type = TaintSourceType::USER_INPUT;
                        taint.source_location = convertSourceLoc(call_expr->location);
                        taint.taint_depth = 0;

                        current_taint.insert(taint);
                        std::cout << "[TAINT] Tainted variable: " << var_name << std::endl;
                    }
                }

                // For gets(buffer) - taint the buffer argument
                // gets has only 1 argument: buffer
                else if (call_expr->arguments.size() >= 1)
                {
                    Expr *buffer_arg = call_expr->arguments[0].get();

                    if (auto var_expr = dynamic_cast<VarExpr *>(buffer_arg))
                    {
                        std::string var_name = var_expr->name;

                        TaintedValue taint;
                        taint.identifier = var_name;
                        taint.source_type = TaintSourceType::USER_INPUT;
                        taint.source_location = convertSourceLoc(call_expr->location);
                        taint.taint_depth = 0;

                        current_taint.insert(taint);
                        std::cout << "[TAINT] Tainted variable: " << var_name << std::endl;
                    }
                }
            }

            // Check if this is a taint SINK (strcpy, sprintf, system)
            if (isTaintSinkCall(call_expr))
            {
                std::cout << "[TAINT] Found taint sink: " << call_expr->funcName << std::endl;

                // Check each argument for taint
                for (size_t i = 0; i < call_expr->arguments.size(); i++)
                {
                    Expr *arg = call_expr->arguments[i].get();
                    if (auto var_arg = dynamic_cast<VarExpr *>(arg))
                    {
                        std::string var_name = var_arg->name;

                        // Check if this variable is in current taint set
                        for (const auto &taint : current_taint)
                        {
                            if (taint.identifier == var_name)
                            {
                                std::cout << "[TAINT] Tainted variable '" << var_name
                                          << "' used in sink: " << call_expr->funcName << std::endl;

                                // We found a taint flow! Store it for reporting
                                // This will be enhanced in Phase F.2.2
                            }
                        }
                    }
                }
            }
        }
        // For other expressions, use the existing analyzeExpression
        else
        {
            analyzeExpression(expr, current_taint);
        }
    }
    // Handle variable declarations with initializers
    else if (auto var_decl = dynamic_cast<VarDecl *>(stmt))
    {
        if (var_decl->initializer)
        {
            Expr *initializer = var_decl->initializer.get();

            // Check if initializer is a VarExpr that might be tainted
            if (auto var_init = dynamic_cast<VarExpr *>(initializer))
            {
                std::string init_name = var_init->name;

                for (const auto &taint : current_taint)
                {
                    if (taint.identifier == init_name)
                    {
                        // Propagate taint to the new variable
                        TaintedValue new_taint = taint;
                        new_taint.identifier = var_decl->varName;
                        new_taint.taint_depth = taint.taint_depth + 1;
                        current_taint.insert(new_taint);
                        std::cout << "[TAINT] Taint propagated: " << init_name
                                  << " -> " << var_decl->varName << std::endl;
                        break;
                    }
                }
            }
            // Check if initializer is a CallExpr that's a taint source
            else if (auto call_init = dynamic_cast<CallExpr *>(initializer))
            {
                if (isTaintSourceCall(call_init))
                {
                    TaintedValue taint;
                    taint.identifier = var_decl->varName;
                    taint.source_type = TaintSourceType::USER_INPUT;
                    taint.source_location = convertSourceLoc(call_init->location);
                    taint.taint_depth = 0;
                    current_taint.insert(taint);
                    std::cout << "[TAINT] Variable initialized with taint: "
                              << var_decl->varName << std::endl;
                }
            }
        }
    }
    // Handle ReturnStmt to check if tainted values are returned
    else if (auto return_stmt = dynamic_cast<ReturnStmt *>(stmt))
    {
        if (return_stmt->value)
        {
            analyzeExpression(return_stmt->value.get(), current_taint);
        }
    }
}

void FunctionContext::propagateTaint(std::shared_ptr<BasicBlock> block)
{
    if (!block)
        return;

    int block_id = block->getId();

    // Get current taint state for this block (start with empty)
    std::unordered_set<TaintedValue, TaintedValueHash> block_taint;

    // Merge taints from all incoming edges
    for (const auto &edge : block->getIncomingEdges())
    {
        auto source_block = edge->getSource();
        int source_id = source_block->getId();

        if (block_taint_state_.find(source_id) != block_taint_state_.end())
        {
            const auto &source_taints = block_taint_state_[source_id];
            block_taint.insert(source_taints.begin(), source_taints.end());
        }
    }

    // Analyze each statement in the block
    size_t original_size = block_taint.size();

    for (const auto &stmt_ptr : block->getStatements())
    {
        // Get raw pointer from shared_ptr
        Stmt *stmt = stmt_ptr.get();
        analyzeStatement(stmt, block_taint);
    }

    bool taint_changed = (block_taint.size() != original_size);

    // Update block's taint state
    block_taint_state_[block_id] = block_taint;

    // If taint changed, add successors to worklist
    if (taint_changed)
    {
        for (const auto &edge : block->getOutgoingEdges())
        {
            auto successor = edge->getTarget();
            // Add to worklist if not already there
            auto it = std::find(worklist_.begin(), worklist_.end(), successor);
            if (it == worklist_.end())
            {
                worklist_.push_back(successor);
            }
        }
    }
}
std::vector<std::pair<TaintedValue, TaintedValue>> FunctionContext::analyze()
{
    std::vector<std::pair<TaintedValue, TaintedValue>> taint_paths;

    if (!cfg_)
        return taint_paths;

    std::cout << "[TAINT] Analyzing function: " << function_name_ << std::endl;

    // Track sources and sinks we find
    std::vector<TaintedValue> taint_sources;
    std::vector<std::pair<TaintedValue, CallExpr *>> taint_sinks;

    // Reset and initialize
    block_taint_state_.clear();
    for (const auto &block : cfg_->getBlocks())
    {
        block_taint_state_[block->getId()] = {};
    }

    // Process all blocks
    for (const auto &block : cfg_->getBlocks())
    {
        int block_id = block->getId();
        std::unordered_set<TaintedValue, TaintedValueHash> block_taint;

        // Merge from incoming edges
        for (const auto &edge : block->getIncomingEdges())
        {
            auto source_block = edge->getSource();
            int source_id = source_block->getId();
            auto it = block_taint_state_.find(source_id);
            if (it != block_taint_state_.end())
            {
                block_taint.insert(it->second.begin(), it->second.end());
            }
        }

        // Process statements
        for (const auto &stmt_ptr : block->getStatements())
        {
            Stmt *stmt = stmt_ptr.get();
            analyzeStatement(stmt, block_taint);

            // Check for taint sources (we could track them here)
            // Check for taint sinks with tainted arguments
            // This logic should be in analyzeStatement, but we can
            // track paths here
        }

        // Save state
        block_taint_state_[block_id] = block_taint;
    }

    // Simple path creation: if we have both sources and sinks in same function
    // This is placeholder - will be enhanced in Phase F.2
    auto sources = getTaintSources();
    if (!sources.empty())
    {
        std::cout << "[TAINT] Found " << sources.size()
                  << " taint source(s) in function '" << function_name_ << "'" << std::endl;
    }

    return taint_paths;
}
std::unordered_set<TaintedValue, TaintedValueHash> FunctionContext::getExitTaints() const
{
    auto exit_block = cfg_->getExitBlock();
    if (!exit_block)
        return {};

    int exit_id = exit_block->getId();
    auto it = block_taint_state_.find(exit_id);
    if (it != block_taint_state_.end())
    {
        return it->second;
    }

    return {};
}

bool FunctionContext::isVariableTainted(const std::string &var_name,
                                        std::shared_ptr<BasicBlock> block) const
{

    // If no specific block, check all blocks
    if (!block)
    {
        for (const auto &pair : block_taint_state_)
        {
            for (const auto &taint : pair.second)
            {
                if (taint.identifier == var_name)
                {
                    return true;
                }
            }
        }
        return false;
    }

    // Check specific block
    int block_id = block->getId();
    auto it = block_taint_state_.find(block_id);
    if (it != block_taint_state_.end())
    {
        for (const auto &taint : it->second)
        {
            if (taint.identifier == var_name)
            {
                return true;
            }
        }
    }

    return false;
}

std::vector<TaintedValue> FunctionContext::getTaintSources() const
{
#if TAINT_DEBUG_ENABLED
    std::cout << "[TAINT-DEBUG] === getTaintSources() called ===" << std::endl;
    std::cout << "[TAINT-DEBUG] Total blocks in state: " << block_taint_state_.size() << std::endl;
#endif

    std::vector<TaintedValue> sources;
    std::unordered_set<std::string> seen_identifiers;

    for (const auto &pair : block_taint_state_)
    {
#if TAINT_DEBUG_ENABLED
        std::cout << "[TAINT-DEBUG] Checking block #" << pair.first
                  << " with " << pair.second.size() << " taints" << std::endl;
#endif

        for (const auto &taint : pair.second)
        {
#if TAINT_DEBUG_ENABLED
            std::cout << "[TAINT-DEBUG]   Taint: '" << taint.identifier
                      << "' depth=" << taint.depth
                      << " source_type=" << static_cast<int>(taint.source_type) << std::endl;
#endif

            if (seen_identifiers.find(taint.identifier) == seen_identifiers.end())
            {
                // This is a new source we haven't seen before
                sources.push_back(taint);
                seen_identifiers.insert(taint.identifier);

#if TAINT_DEBUG_ENABLED
                std::cout << "[TAINT-DEBUG]   -> Adding to sources!" << std::endl;
#endif
            }
#if TAINT_DEBUG_ENABLED
            else
            {
                std::cout << "[TAINT-DEBUG]   -> Already seen, skipping" << std::endl;
            }
#endif
        }
    }

#if TAINT_DEBUG_ENABLED
    std::cout << "[TAINT-DEBUG] Returning " << sources.size() << " sources" << std::endl;
    std::cout << "[TAINT-DEBUG] === getTaintSources() end ===" << std::endl;
#endif

    return sources;
}

std::string FunctionContext::getTaintGraph() const
{
    std::stringstream result;
    result << "Taint Graph for function: " << function_name_ << "\n";

    for (const auto &block : cfg_->getBlocks())
    {
        int block_id = block->getId();
        result << "Block #" << block_id << ":\n";

        auto it = block_taint_state_.find(block_id);
        if (it != block_taint_state_.end())
        {
            for (const auto &taint : it->second)
            {
                result << "  - " << taint.identifier
                       << " (source: " << static_cast<int>(taint.source_type)
                       << ", depth: " << taint.taint_depth << ")\n";
            }
        }
        else
        {
            result << "  No taints\n";
        }
    }

    return result.str();
}