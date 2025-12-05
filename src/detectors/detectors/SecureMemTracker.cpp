// src/detectors/detectors/SecureMemTracker.cpp - CLEANED UP
#include "SecureMemTracker.hpp"
#include "../../ast/ast_nodes.h"
#include "../../semantic/SymbolTable.hpp"
#include <algorithm>

namespace detectors
{

    SecureMemTracker::SecureMemTracker()
        : VulnerabilityDetector("MEM001", "Unsafe memory/string function usage"),
          ast_(nullptr),
          symtab_(nullptr),
          unsafe_functions_{"strcpy", "gets", "sprintf", "strcat", "scanf", "printf"} {}

    std::vector<Finding> SecureMemTracker::analyze(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        ast_ = ast;
        symtab_ = symtab;
        findings_.clear();

        // Handle both Program and CompoundStmt as root
        if (auto program = dynamic_cast<Program *>(ast.get()))
        {
            visitProgram(program);
        }
        else if (auto compound = dynamic_cast<CompoundStmt *>(ast.get()))
        {
            visitCompoundStmt(compound);
        }

        return std::move(findings_);
    }

    void SecureMemTracker::visitProgram(Program *node)
    {
        if (!node)
            return;

        for (size_t i = 0; i < node->getDeclarationCount(); ++i)
        {
            Stmt *stmt = node->getDeclaration(i);

            if (auto funcDecl = dynamic_cast<FunctionDecl *>(stmt))
            {
                visitFunctionDecl(funcDecl);
            }
        }
    }

    void SecureMemTracker::visitFunctionDecl(FunctionDecl *node)
    {
        if (!node)
            return;

        if (node->getBody())
        {
            visitCompoundStmt(node->getBody());
        }
    }

    void SecureMemTracker::visitCompoundStmt(CompoundStmt *node)
    {
        if (!node)
            return;

        for (size_t i = 0; i < node->getStatementCount(); ++i)
        {
            Stmt *stmt = node->getStatement(i);
            if (!stmt)
                continue;

            if (auto ifStmt = dynamic_cast<IfStmt *>(stmt))
            {
                // Can add traversal here if needed
            }
            else if (auto whileStmt = dynamic_cast<WhileStmt *>(stmt))
            {
                // Can add traversal here if needed
            }
            else if (auto returnStmt = dynamic_cast<ReturnStmt *>(stmt))
            {
                // Check return statements
            }
            else if (auto exprStmt = dynamic_cast<ExprStmt *>(stmt))
            {
                visitExprStmt(exprStmt);
            }
            else if (auto varDecl = dynamic_cast<VarDecl *>(stmt))
            {
                // Check variable declarations
            }
            else if (auto compoundStmt = dynamic_cast<CompoundStmt *>(stmt))
            {
                visitCompoundStmt(compoundStmt);
            }
            else if (auto funcDecl = dynamic_cast<FunctionDecl *>(stmt))
            {
                visitFunctionDecl(funcDecl);
            }
        }
    }

    void SecureMemTracker::visitExprStmt(ExprStmt *node)
    {
        if (!node)
            return;

        Expr *expr = node->getExpression();
        if (auto callExpr = dynamic_cast<CallExpr *>(expr))
        {
            checkUnsafeFunctionCall(callExpr);
        }
    }

    void SecureMemTracker::checkUnsafeFunctionCall(CallExpr *callExpr)
    {
        if (!callExpr)
            return;

        std::string funcName = callExpr->getFunctionName();

        auto it = std::find(unsafe_functions_.begin(), unsafe_functions_.end(), funcName);
        if (it != unsafe_functions_.end())
        {
            SourceLocation loc(callExpr->location.line,
                               callExpr->location.column,
                               "");

            Finding finding = createBaseFinding(loc, Severity::HIGH,
                                                "Use of unsafe function: " + funcName);

            if (funcName == "strcpy")
            {
                finding.cert_reference = "CERT-C STR00-C";
                finding.owasp_reference = "OWASP A8:2017";
            }
            else if (funcName == "gets")
            {
                finding.cert_reference = "CERT-C FIO00-C";
                finding.owasp_reference = "OWASP A9:2017";
            }
            else if (funcName == "sprintf")
            {
                finding.cert_reference = "CERT-C FIO00-C";
                finding.owasp_reference = "OWASP A8:2017";
            }
            else if (funcName == "strcat")
            {
                finding.cert_reference = "CERT-C STR00-C";
                finding.owasp_reference = "OWASP A8:2017";
            }
            else if (funcName == "scanf")
            {
                finding.cert_reference = "CERT-C FIO00-C";
                finding.owasp_reference = "OWASP A1:2017";
            }
            else if (funcName == "printf")
            {
                finding.cert_reference = "CERT-C FIO00-C";
                finding.owasp_reference = "OWASP A1:2017";
            }

            findings_.push_back(finding);
        }
    }

} // namespace detectors