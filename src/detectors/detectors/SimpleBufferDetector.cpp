// src/detectors/detectors/SimpleBufferDetector.cpp
#include "SimpleBufferDetector.hpp"

namespace detectors
{

    SimpleBufferDetector::SimpleBufferDetector()
        : VulnerabilityDetector("BUF001",
                                "Buffer Overflow Heuristic Detection")
    {
        config_.enabled = true;
        config_.severity_override = Severity::HIGH;
        config_.risk_weight = 2;
    }

    std::vector<Finding> SimpleBufferDetector::analyze(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        std::vector<Finding> findings;

        if (!ast || !config_.enabled)
        {
            return findings;
        }

        // Handle different AST types
        if (auto funcDecl = std::dynamic_pointer_cast<FunctionDecl>(ast))
        {
            analyzeFunction(funcDecl.get(), findings, symtab);
        }
        else if (auto program = std::dynamic_pointer_cast<Program>(ast))
        {
            for (size_t i = 0; i < program->getDeclarationCount(); i++)
            {
                if (auto decl = program->getDeclaration(i))
                {
                    if (auto func = dynamic_cast<FunctionDecl *>(decl))
                    {
                        analyzeFunction(func, findings, symtab);
                    }
                }
            }
        }

        return findings;
    }

    void SimpleBufferDetector::analyzeFunction(FunctionDecl *funcDecl,
                                               std::vector<Finding> &findings,
                                               SymbolTable *symtab)
    {
        if (!funcDecl || !funcDecl->hasBody())
        {
            return;
        }

        analyzeStatement(funcDecl->getBody(), findings, symtab);
    }

    void SimpleBufferDetector::analyzeStatement(Stmt *stmt,
                                                std::vector<Finding> &findings,
                                                SymbolTable *symtab)
    {
        if (!stmt)
            return;

        // Check for loops that might cause buffer overflows
        if (auto whileStmt = dynamic_cast<WhileStmt *>(stmt))
        {
            checkLoopBounds(whileStmt, findings, symtab);
            analyzeExpression(whileStmt->getCondition(), findings, symtab);
            analyzeStatement(whileStmt->getBody(), findings, symtab);
        }
        else if (auto ifStmt = dynamic_cast<IfStmt *>(stmt))
        {
            analyzeExpression(ifStmt->getCondition(), findings, symtab);
            analyzeStatement(ifStmt->getThenBranch(), findings, symtab);
            if (ifStmt->hasElseBranch())
            {
                analyzeStatement(ifStmt->getElseBranch(), findings, symtab);
            }
        }
        else if (auto compound = dynamic_cast<CompoundStmt *>(stmt))
        {
            for (size_t i = 0; i < compound->getStatementCount(); i++)
            {
                analyzeStatement(compound->getStatement(i), findings, symtab);
            }
        }
        else if (auto exprStmt = dynamic_cast<ExprStmt *>(stmt))
        {
            analyzeExpression(exprStmt->getExpression(), findings, symtab);
        }
        else if (auto returnStmt = dynamic_cast<ReturnStmt *>(stmt))
        {
            if (returnStmt->hasValue())
            {
                analyzeExpression(returnStmt->getValue(), findings, symtab);
            }
        }
    }

    void SimpleBufferDetector::analyzeExpression(Expr *expr,
                                                 std::vector<Finding> &findings,
                                                 SymbolTable *symtab)
    {
        if (!expr)
            return;

        // Check binary expressions for array-like access
        if (auto binaryExpr = dynamic_cast<BinaryExpr *>(expr))
        {
            // Check for array subscript pattern
            analyzeExpression(binaryExpr->getLeft(), findings, symtab);
            analyzeExpression(binaryExpr->getRight(), findings, symtab);
        }
        // Check function calls for buffer operations
        else if (auto callExpr = dynamic_cast<CallExpr *>(expr))
        {
            std::string funcName = callExpr->getFunctionName();

            // Check for memory/array manipulation functions
            if (funcName.find("mem") != std::string::npos ||
                funcName.find("str") != std::string::npos ||
                funcName.find("cpy") != std::string::npos ||
                funcName.find("cat") != std::string::npos)
            {

                // Heuristic: functions with "mem", "str", "cpy", "cat" might be dangerous
                std::string desc = "Potential buffer operation: '" + funcName + "'";
                SourceLocation loc(callExpr->location.line, callExpr->location.column);

                auto finding = createBaseFinding(loc, Severity::MEDIUM, desc);
                finding.function_name = funcName;
                findings.push_back(finding);
            }

            // Analyze arguments
            for (size_t i = 0; i < callExpr->getArgCount(); i++)
            {
                analyzeExpression(callExpr->getArgument(i), findings, symtab);
            }
        }
        // Check unary expressions (pointer dereference)
        else if (auto unaryExpr = dynamic_cast<UnaryExpr *>(expr))
        {
            analyzeExpression(unaryExpr->getOperand(), findings, symtab);
        }
    }

    void SimpleBufferDetector::checkLoopBounds(WhileStmt *whileStmt,
                                               std::vector<Finding> &findings,
                                               SymbolTable *symtab)
    {
        if (!whileStmt)
            return;

        // Simple heuristic: loops might cause buffer overflows
        // In a real implementation, we would analyze the condition
        // to see if it could exceed array bounds

        std::string desc = "Loop may cause buffer overflow (heuristic check)";
        SourceLocation loc(whileStmt->location.line, whileStmt->location.column);

        auto finding = createBaseFinding(loc, Severity::LOW, desc);
        findings.push_back(finding);
    }

} // namespace detectors