// src/detectors/detectors/TaintAnalysis.hpp
#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "ast/ast_nodes.h"
#include "cfg/BasicBlock.hpp"
#include "utils/SourceLocation.hpp"

namespace detectors
{

    /**
     * @brief Core taint analysis engine
     */
    class TaintAnalysis
    {
    public:
        struct TaintNode
        {
            std::string identifier;
            SourceLocation location;
            int depth;
            std::vector<std::shared_ptr<TaintNode>> predecessors;
        };

        TaintAnalysis();

        /**
         * @brief Check if an expression is a taint source
         */
        bool isTaintSource(std::shared_ptr<Expr> expr) const;

        /**
         * @brief Check if an expression is a taint sink
         */
        bool isTaintSink(std::shared_ptr<Expr> expr) const;

        /**
         * @brief Propagate taint through an expression
         */
        std::unordered_set<std::string> propagateTaint(
            std::shared_ptr<Expr> expr,
            const std::unordered_set<std::string> &input_taints);

        /**
         * @brief Get all tainted variables in an expression
         */
        std::unordered_set<std::string> getTaintedVariables(
            std::shared_ptr<Expr> expr,
            const std::unordered_set<std::string> &current_taints);

        /**
         * @brief Build taint propagation graph
         */
        std::shared_ptr<TaintNode> buildTaintGraph(
            const std::string &start_var,
            std::shared_ptr<BasicBlock> block);

    private:
        std::unordered_set<std::string> taint_sources_;
        std::unordered_set<std::string> taint_sinks_;
        std::unordered_set<std::string> sanitizers_;

        void initializeSourcesAndSinks();
    };

} // namespace detectors