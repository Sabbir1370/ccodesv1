// src/detectors/detectors/TaintFlowDetector.hpp
#pragma once

#include "../VulnerabilityDetector.hpp"
#include "FunctionContext.hpp"
#include <memory>
#include <vector>
#include <string>

namespace detectors
{

    class TaintFlowDetector : public VulnerabilityDetector
    {
    private:
        struct TaintPath
        {
            TaintedValue source;
            TaintedValue sink;
            std::vector<SourceLocation> propagation_path;
            std::string function_name;

            std::string toString() const
            {
                return "Taint from " + source.identifier + " at " +
                       source.source_location.toString() +
                       " to " + sink.identifier + " at " +
                       sink.source_location.toString();
            }
        };

        std::vector<TaintPath> detected_paths_;

        // Configuration
        bool track_intra_procedural_ = true;
        bool track_pointers_ = true;
        int max_taint_depth_ = 10;

        void analyzeFunction(std::shared_ptr<FunctionDecl> func_decl,
                             std::shared_ptr<CFG> cfg,
                             SymbolTable *symtab);

        Finding createFindingFromTaintPath(const TaintPath &path);

    public:
        TaintFlowDetector();

        std::vector<Finding> analyze(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs) override;

        // Configuration setters
        void setTrackPointers(bool track) { track_pointers_ = track; }
        void setMaxTaintDepth(int depth) { max_taint_depth_ = depth; }

        // Statistics
        size_t getDetectedPathCount() const { return detected_paths_.size(); }
        const std::vector<TaintPath> &getDetectedPaths() const { return detected_paths_; }
    };

} // namespace detectors