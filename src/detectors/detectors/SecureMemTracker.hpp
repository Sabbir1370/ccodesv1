// src/detectors/detectors/SecureMemTracker.hpp - FIXED
#ifndef SECURE_MEM_TRACKER_HPP
#define SECURE_MEM_TRACKER_HPP

#include "../VulnerabilityDetector.hpp"
#include <memory>
#include <vector>
#include <string>

// Forward declarations in GLOBAL namespace
class ASTNode;
class Program;
class Stmt;
class Expr;
class CallExpr;
class FunctionDecl;
class CompoundStmt;
class ExprStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class VarDecl;
class SymbolTable;
class CFG; // CFG is in global namespace

namespace detectors
{

    class SecureMemTracker : public VulnerabilityDetector
    {
    private:
        std::shared_ptr<ASTNode> ast_;
        SymbolTable *symtab_;
        std::vector<Finding> findings_;

        const std::vector<std::string> unsafe_functions_;

        void checkUnsafeFunctionCall(CallExpr *callExpr);
        void visitProgram(Program *node);
        void visitFunctionDecl(FunctionDecl *node);
        void visitCompoundStmt(CompoundStmt *node);
        void visitExprStmt(ExprStmt *node);

    public:
        SecureMemTracker();

        std::vector<Finding> analyze(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs // CFG, not cfg::CFG
            ) override;
    };

} // namespace detectors

#endif // SECURE_MEM_TRACKER_HPP