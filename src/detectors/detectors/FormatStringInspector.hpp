// src/detectors/detectors/FormatStringInspector.hpp
#pragma once

#include "../VulnerabilityDetector.hpp"
#include "ast/ast_nodes.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>

namespace detectors
{

    class FormatStringInspector : public VulnerabilityDetector
    {
    private:
        // Common format string functions to check
        std::unordered_set<std::string> formatFunctions = {
            "printf", "scanf", "sprintf", "snprintf", "fprintf",
            "fscanf", "sscanf", "vprintf", "vfprintf", "vsprintf"};

        // Format specifier patterns
        std::unordered_set<std::string> formatSpecifiers = {
            "%d", "%i", "%u", "%f", "%lf", "%c", "%s", "%p", "%x", "%o", "%n"};

        // Helper methods
        void analyzeCallExpression(CallExpr *callExpr,
                                   std::vector<Finding> &findings,
                                   SymbolTable *symtab);

        bool isFormatStringFunction(const std::string &funcName);

        void checkFormatString(CallExpr *callExpr,
                               Expr *formatArg,
                               const std::vector<Expr *> &otherArgs,
                               std::vector<Finding> &findings);

        int countFormatSpecifiers(const std::string &formatStr);
        std::string getExpectedArgType(const std::string &specifier);

    public:
        FormatStringInspector();

        std::vector<Finding> analyze(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs) override;

        // Configuration
        void addFormatFunction(const std::string &funcName)
        {
            formatFunctions.insert(funcName);
        }

        void removeFormatFunction(const std::string &funcName)
        {
            formatFunctions.erase(funcName);
        }
    };

} // namespace detectors