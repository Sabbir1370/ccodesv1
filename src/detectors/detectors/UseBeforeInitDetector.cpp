// src/detectors/detectors/UseBeforeInitDetector.cpp - FIXED VERSION
#include "UseBeforeInitDetector.hpp"
#include <iostream>

namespace detectors
{

    UseBeforeInitDetector::UseBeforeInitDetector()
        : VulnerabilityDetector("INIT001",
                                "Use of Uninitialized Variables Detection")
    {
        // Default configuration
        config_.enabled = true;
        config_.severity_override = Severity::HIGH;
        config_.risk_weight = 2;
    }

    std::vector<Finding> UseBeforeInitDetector::analyze(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        std::vector<Finding> findings;

        if (!ast || !config_.enabled)
        {
            return findings;
        }

        // std::cout << "[INIT001-DEBUG] === START ANALYSIS ===" << std::endl;

        // Check what type of node we got
        // std::cout << "[INIT001-DEBUG] Node type check..." << std::endl;

        // FIRST try Program (top-level with function declarations)
        if (auto program = std::dynamic_pointer_cast<Program>(ast))
        {
            // std::cout << "[INIT001-DEBUG] Got Program with "
            // << program->getDeclarationCount() << " declarations" << std::endl;

            // Each declaration should be a FunctionDecl
            for (size_t i = 0; i < program->getDeclarationCount(); i++)
            {
                if (auto decl = program->getDeclaration(i))
                {
                    if (auto func = dynamic_cast<FunctionDecl *>(decl))
                    {
                        // std::cout << "[INIT001-DEBUG] Analyzing function: "
                        // << func->getFunctionName() << std::endl;
                        // analyzeFunction(func, findings, symtab);
                    }
                    else
                    {
                        // std::cout << "[INIT001-DEBUG] Declaration is not a FunctionDecl" << std::endl;
                    }
                }
            }
        }
        // If not Program, try CompoundStmt (function body)
        else if (auto compound = std::dynamic_pointer_cast<CompoundStmt>(ast))
        {
            // std::cout << "[INIT001-DEBUG] Got CompoundStmt directly" << std::endl;

            // This is a function body - analyze it
            AnalysisState state;
            analyzeStatement(compound.get(), state, findings, symtab);
        }
        // Try FunctionDecl directly
        else if (auto funcDecl = std::dynamic_pointer_cast<FunctionDecl>(ast))
        {
            // std::cout << "[INIT001-DEBUG] Got FunctionDecl directly: "
            // << funcDecl->getFunctionName() << std::endl;
            // analyzeFunction(funcDecl.get(), findings, symtab);
        }
        else
        {
            // std::cout << "[INIT001-DEBUG] Unknown AST type: " << typeid(*ast).name() << std::endl;
        }

        // std::cout << "[INIT001-DEBUG] Found " << findings.size() << " issues" << std::endl;
        // std::cout << "[INIT001-DEBUG] === END ANALYSIS ===" << std::endl;

        return findings;
    }
    void UseBeforeInitDetector::analyzeFunction(FunctionDecl *funcDecl,
                                                std::vector<Finding> &findings,
                                                SymbolTable *symtab)
    {
        if (!funcDecl || !funcDecl->hasBody())
        {
            return;
        }

        // std::cout << "[INIT001-DEBUG] Analyzing function: "
        // << funcDecl->getFunctionName() << std::endl;

        AnalysisState state;

        // Initialize parameter states as initialized
        for (size_t i = 0; i < funcDecl->getParamCount(); i++)
        {
            if (auto param = funcDecl->getParameter(i))
            {
                // std::cout << "[INIT001-DEBUG] Parameter: " << param->getVarName()
                // << " marked as INITIALIZED" << std::endl;
                // updateVarState(param->getVarName(), InitState::INITIALIZED, state);
            }
        }

        // Analyze function body
        analyzeStatement(funcDecl->getBody(), state, findings, symtab);
    }

    void UseBeforeInitDetector::analyzeStatement(Stmt *stmt,
                                                 AnalysisState &state,
                                                 std::vector<Finding> &findings,
                                                 SymbolTable *symtab)
    {
        if (!stmt)
            return;

        // std::cout << "[INIT001-DEBUG] Statement at line " << stmt->location.line
        // << ", type: " << typeid(*stmt).name() << std::endl;

        // Handle FunctionDecl (nested function declaration - shouldn't happen in C)
        if (auto funcDecl = dynamic_cast<FunctionDecl *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   FunctionDecl: " << funcDecl->getFunctionName() << std::endl;
            // Analyze this function
            analyzeFunction(funcDecl, findings, symtab);
            return;
        }
        // Handle different statement types
        if (auto varDecl = dynamic_cast<VarDecl *>(stmt))
        {
            // Variable declaration with/without initializer
            std::string varName = varDecl->getVarName();
            // std::cout << "[INIT001-DEBUG]   VarDecl: " << varName;

            if (varDecl->hasInitializer())
            {
                // std::cout << " has initializer" << std::endl;
                // Analyze initializer expression
                analyzeExpression(varDecl->getInitializer(), state, findings, symtab, false);
                // Mark variable as initialized
                updateVarState(varName, InitState::INITIALIZED, state);
            }
            else
            {
                // std::cout << " NO initializer" << std::endl;
                // No initializer - variable is uninitialized
                updateVarState(varName, InitState::UNINITIALIZED, state);
            }
        }
        else if (auto exprStmt = dynamic_cast<ExprStmt *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   ExprStmt" << std::endl;
            // Expression statement (could be assignment or function call)
            analyzeExpression(exprStmt->getExpression(), state, findings, symtab, false);
        }
        else if (auto compound = dynamic_cast<CompoundStmt *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   CompoundStmt with "
            // << compound->getStatementCount() << " statements" << std::endl;
            // Compound statement - analyze all statements in order
            for (size_t i = 0; i < compound->getStatementCount(); i++)
            {
                analyzeStatement(compound->getStatement(i), state, findings, symtab);
            }
        }
        else if (auto ifStmt = dynamic_cast<IfStmt *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   IfStmt" << std::endl;
            // If statement - need to analyze both branches

            // Save current state
            AnalysisState savedState = state;

            // Analyze condition
            analyzeExpression(ifStmt->getCondition(), state, findings, symtab);

            // Analyze then branch
            analyzeStatement(ifStmt->getThenBranch(), state, findings, symtab);

            // Analyze else branch if it exists
            if (ifStmt->hasElseBranch())
            {
                // Restore state before then branch
                AnalysisState elseState = savedState;
                analyzeExpression(ifStmt->getCondition(), elseState, findings, symtab);
                analyzeStatement(ifStmt->getElseBranch(), elseState, findings, symtab);

                // Merge states (variables might be initialized in either branch)
                for (auto &[varName, elseStateVal] : elseState.varStates)
                {
                    auto it = state.varStates.find(varName);
                    if (it != state.varStates.end())
                    {
                        if (it->second != elseStateVal)
                        {
                            // Different states in different branches
                            it->second = InitState::MAYBE_INITIALIZED;
                        }
                    }
                }
            }
        }
        else if (auto whileStmt = dynamic_cast<WhileStmt *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   WhileStmt" << std::endl;
            // While loop - analyze condition and body
            analyzeExpression(whileStmt->getCondition(), state, findings, symtab);
            analyzeStatement(whileStmt->getBody(), state, findings, symtab);

            // After loop, variables might not be initialized if loop never executes
            // For simplicity, we'll mark as MAYBE
            for (auto &[varName, varState] : state.varStates)
            {
                if (varState == InitState::INITIALIZED)
                {
                    varState = InitState::MAYBE_INITIALIZED;
                }
            }
        }
        else if (auto returnStmt = dynamic_cast<ReturnStmt *>(stmt))
        {
            // std::cout << "[INIT001-DEBUG]   ReturnStmt" << std::endl;
            if (returnStmt->hasValue())
            {
                analyzeExpression(returnStmt->getValue(), state, findings, symtab);
            }
        }
        else
        {
            // std::cout << "[INIT001-DEBUG]   Unknown statement type" << std::endl;
        }
    }

    void UseBeforeInitDetector::analyzeExpression(Expr *expr,
                                                  AnalysisState &state,
                                                  std::vector<Finding> &findings,
                                                  SymbolTable *symtab,
                                                  bool isReadContext)
    {
        if (!expr)
            return;

        // std::cout << "[INIT001-DEBUG]   Expression at line " << expr->location.line << std::endl;

        if (auto varExpr = dynamic_cast<VarExpr *>(expr))
        {
            // Variable usage
            // std::cout << "[INIT001-DEBUG]     VarExpr: " << varExpr->getName()
            // << " (readContext: " << isReadContext << ")" << std::endl;

            if (isReadContext)
            {
                // Convert SourceLoc to SourceLocation
                SourceLocation loc(varExpr->location.line, varExpr->location.column);
                checkVariableUse(varExpr->getName(), loc, state, findings);
            }
        }
        else if (auto binaryExpr = dynamic_cast<BinaryExpr *>(expr))
        {
            // std::cout << "[INIT001-DEBUG]     BinaryExpr" << std::endl;
            // Binary expression
            analyzeExpression(binaryExpr->getLeft(), state, findings, symtab, isReadContext);
            analyzeExpression(binaryExpr->getRight(), state, findings, symtab, isReadContext);
        }
        else if (auto unaryExpr = dynamic_cast<UnaryExpr *>(expr))
        {
            // std::cout << "[INIT001-DEBUG]     UnaryExpr" << std::endl;
            // Unary expression
            analyzeExpression(unaryExpr->getOperand(), state, findings, symtab, isReadContext);
        }
        else if (auto callExpr = dynamic_cast<CallExpr *>(expr))
        {
            // std::cout << "[INIT001-DEBUG]     CallExpr: " << callExpr->getFunctionName()
            // << " with " << callExpr->getArgCount() << " args" << std::endl;
            // Function call - analyze arguments
            for (size_t i = 0; i < callExpr->getArgCount(); i++)
            {
                analyzeExpression(callExpr->getArgument(i), state, findings, symtab, true);
            }
        }
        else if (auto literal = dynamic_cast<LiteralExpr *>(expr))
        {
            // std::cout << "[INIT001-DEBUG]     LiteralExpr" << std::endl;
            // Literal - nothing to check
        }
        else
        {
            // std::cout << "[INIT001-DEBUG]     Unknown expression type" << std::endl;
        }
    }

    void UseBeforeInitDetector::updateVarState(const std::string &varName,
                                               InitState newState,
                                               AnalysisState &state)
    {
        // std::cout << "[INIT001-DEBUG]   Update var '" << varName
        // << "' to state " << static_cast<int>(newState) << std::endl;
        state.varStates[varName] = newState;
    }

    UseBeforeInitDetector::InitState UseBeforeInitDetector::getVarState(
        const std::string &varName,
        const AnalysisState &state) const
    {
        auto it = state.varStates.find(varName);
        if (it != state.varStates.end())
        {
            return it->second;
        }
        return InitState::UNINITIALIZED; // Default to uninitialized
    }

    void UseBeforeInitDetector::checkVariableUse(const std::string &varName,
                                                 const SourceLocation &location,
                                                 AnalysisState &state,
                                                 std::vector<Finding> &findings)
    {
        // std::cout << "[INIT001-DEBUG]   Checking variable '" << varName << "'" << std::endl;

        // Check if we've already reported this variable in this context
        if (state.reportedVars.find(varName) != state.reportedVars.end())
        {
            // std::cout << "[INIT001-DEBUG]     Already reported, skipping" << std::endl;
            return;
        }

        InitState currentState = getVarState(varName, state);
        // std::cout << "[INIT001-DEBUG]     Current state: " << static_cast<int>(currentState) << std::endl;

        if (currentState == InitState::UNINITIALIZED)
        {
            // Create finding for use of uninitialized variable
            std::string desc = "Use of uninitialized variable: '" + varName + "'";

            // std::cout << "[INIT001-DEBUG]     CREATING FINDING!" << std::endl;

            auto finding = createBaseFinding(location, Severity::HIGH, desc);
            finding.variable_name = varName;
            findings.push_back(finding);

            // Mark as reported to avoid duplicate findings
            state.reportedVars.insert(varName);
        }
        else if (currentState == InitState::MAYBE_INITIALIZED)
        {
            // Variable might be uninitialized
            std::string desc = "Variable '" + varName + "' might be uninitialized";

            auto finding = createBaseFinding(location, Severity::MEDIUM, desc);
            finding.variable_name = varName;
            findings.push_back(finding);

            state.reportedVars.insert(varName);
        }
    }

} // namespace detectors