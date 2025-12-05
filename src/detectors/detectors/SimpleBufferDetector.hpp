// src/detectors/detectors/SimpleBufferDetector.hpp
#pragma once

#include "../VulnerabilityDetector.hpp"
#include "ast/ast_nodes.h"
#include <memory>
#include <vector>
#include <string>

namespace detectors
{

    class SimpleBufferDetector : public VulnerabilityDetector
    {
    private:
        // Helper methods
        void analyzeFunction(FunctionDecl *funcDecl,
                             std::vector<Finding> &findings,
                             SymbolTable *symtab);

        void analyzeStatement(Stmt *stmt,
                              std::vector<Finding> &findings,
                              SymbolTable *symtab);

        void analyzeExpression(Expr *expr,
                               std::vector<Finding> &findings,
                               SymbolTable *symtab);

        // Check for common buffer issues
        void checkArrayAccess(Expr *arrayExpr,
                              const SourceLocation &location,
                              std::vector<Finding> &findings);

        void checkLoopBounds(WhileStmt *whileStmt,
                             std::vector<Finding> &findings,
                             SymbolTable *symtab);

    public:
        SimpleBufferDetector();

        std::vector<Finding> analyze(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs) override;
    };

} // namespace detectors