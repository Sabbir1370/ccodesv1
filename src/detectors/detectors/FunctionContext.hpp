// src/detectors/detectors/FunctionContext.hpp - UPDATED
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "ast/ast_nodes.h"
#include "cfg/BasicBlock.hpp"
#include "cfg/CFG.hpp"
#include "utils/SourceLocation.hpp"

namespace detectors
{

    // Forward declaration
    class TaintState;

    /**
     * @brief Taint source types
     */
    enum class TaintSourceType
    {
        USER_INPUT,  // scanf, fgets, read, gets
        ENVIRONMENT, // getenv, argv
        NETWORK,     // recv, read from socket
        FILE_IO,     // fread, fgets from file
        UNTRUSTED    // Other untrusted sources
    };

    /**
     * @brief Taint sink types
     */
    enum class TaintSinkType
    {
        MEMORY_OPERATION,  // strcpy, memcpy, sprintf
        COMMAND_EXECUTION, // system, popen, exec
        FORMAT_STRING,     // printf, sprintf with user format
        SQL_INJECTION,     // SQL query construction
        PATH_TRAVERSAL     // File path operations
    };

    /**
     * @brief Represents a tainted variable or expression
     */
    struct TaintedValue
    {
        std::string identifier;          // Variable name or expression
        TaintSourceType source_type;     // How it became tainted
        SourceLocation source_location;  // Where taint originated
        std::shared_ptr<Expr> expr_node; // AST node if available
        int taint_depth = 0;             // How many propagations deep

        bool operator==(const TaintedValue &other) const
        {
            return identifier == other.identifier;
        }
    };

    /**
     * @brief Hash for TaintedValue to use in unordered_set
     */
    struct TaintedValueHash
    {
        size_t operator()(const TaintedValue &tv) const
        {
            return std::hash<std::string>()(tv.identifier);
        }
    };

    /**
     * @brief Manages taint state within a single function
     */
    class FunctionContext
    {
    private:
        std::shared_ptr<CFG> cfg_;
        std::string function_name_;

        // Taint state at each program point
        std::unordered_map<int, std::unordered_set<TaintedValue, TaintedValueHash>> block_taint_state_;

        // Worklist for iterative data flow analysis
        std::vector<std::shared_ptr<BasicBlock>> worklist_;

        // Taint sources and sinks configuration
        std::unordered_set<std::string> taint_sources_ = {
            "scanf", "fscanf", "sscanf",
            "gets", "fgets",
            "read", "recv",
            "getenv", "getchar", "fgetc"};

        std::unordered_set<std::string> taint_sinks_ = {
            "strcpy", "strcat",
            "sprintf", "vsprintf",
            "system", "popen", "execl", "execv",
            "printf", "fprintf", "sprintf"};

        // Helper method to convert SourceLoc to SourceLocation
        SourceLocation convertSourceLoc(const SourceLoc &loc) const
        {
            return SourceLocation(loc.line, loc.column);
        }

        // Helper methods
        void initializeWorklist();
        void propagateTaint(std::shared_ptr<BasicBlock> block);
        void analyzeStatement(Stmt *stmt,
                              std::unordered_set<TaintedValue, TaintedValueHash> &current_taint);
        void analyzeExpression(Expr *expr,
                               std::unordered_set<TaintedValue, TaintedValueHash> &current_taint);

        // Taint operations
        void addTaint(const std::string &identifier,
                      TaintSourceType source_type,
                      const SourceLocation &location,
                      std::shared_ptr<Expr> expr = nullptr);

        void propagateTaintThroughAssignment(Expr *lhs,
                                             Expr *rhs,
                                             std::unordered_set<TaintedValue, TaintedValueHash> &current_taint);

        bool isTaintSourceCall(CallExpr *call);
        bool isTaintSinkCall(CallExpr *call);

    public:
        FunctionContext(std::shared_ptr<CFG> cfg);

        /**
         * @brief Run taint analysis on the function
         * @return Vector of taint findings (source to sink paths)
         */
        std::vector<std::pair<TaintedValue, TaintedValue>> analyze();

        /**
         * @brief Get tainted values at exit of function
         */
        std::unordered_set<TaintedValue, TaintedValueHash> getExitTaints() const;

        /**
         * @brief Check if a variable is tainted at a specific point
         */
        bool isVariableTainted(const std::string &var_name,
                               std::shared_ptr<BasicBlock> block = nullptr) const;

        /**
         * @brief Get all taint sources in the function
         */
        std::vector<TaintedValue> getTaintSources() const;

        /**
         * @brief Get visualization of taint propagation
         */
        std::string getTaintGraph() const;
    };

} // namespace detectors