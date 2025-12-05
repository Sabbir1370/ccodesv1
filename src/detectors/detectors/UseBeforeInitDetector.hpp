// src/detectors/detectors/UseBeforeInitDetector.hpp
#pragma once

#include "../VulnerabilityDetector.hpp"
#include "ast/ast_nodes.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace detectors
{

    class UseBeforeInitDetector : public VulnerabilityDetector
    {
    private:
        // Tracks initialization state of variables
        enum class InitState
        {
            UNINITIALIZED,
            INITIALIZED,
            MAYBE_INITIALIZED // e.g., in one branch of if-else
        };

        // Analysis state
        struct AnalysisState
        {
            std::unordered_map<std::string, InitState> varStates;
            std::unordered_set<std::string> reportedVars;
        };

        // Helper methods
        void analyzeFunction(FunctionDecl *funcDecl,
                             std::vector<Finding> &findings,
                             SymbolTable *symtab);

        void analyzeStatement(Stmt *stmt,
                              AnalysisState &state,
                              std::vector<Finding> &findings,
                              SymbolTable *symtab);

        void analyzeExpression(Expr *expr,
                               AnalysisState &state,
                               std::vector<Finding> &findings,
                               SymbolTable *symtab,
                               bool isReadContext = true);

        void updateVarState(const std::string &varName,
                            InitState newState,
                            AnalysisState &state);

        InitState getVarState(const std::string &varName,
                              const AnalysisState &state) const;

        void checkVariableUse(const std::string &varName,
                              const SourceLocation &location,
                              AnalysisState &state,
                              std::vector<Finding> &findings);

        // Configuration
        bool track_local_vars_ = true;
        bool track_global_vars_ = false;
        bool track_pointers_ = true;

    public:
        UseBeforeInitDetector();

        std::vector<Finding> analyze(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs) override;

        // Configuration
        void setTrackLocalVars(bool track) { track_local_vars_ = track; }
        void setTrackGlobalVars(bool track) { track_global_vars_ = track; }
        void setTrackPointers(bool track) { track_pointers_ = track; }
    };

} // namespace detectors