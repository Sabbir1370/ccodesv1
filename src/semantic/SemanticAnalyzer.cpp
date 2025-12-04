// src/semantic/SemanticAnalyzer.cpp
#include "semantic/SemanticAnalyzer.hpp"
#include "ast/ast_nodes.h"
#include <iostream>
#include <sstream>

SemanticAnalyzer::SemanticAnalyzer()
    : symbolTable(std::make_unique<SymbolTable>()),
      inFunction(false),
      currentFunction(nullptr)
{
    typeChecker = std::make_unique<TypeChecker>(symbolTable.get());
    initTracker = std::make_unique<InitializationTracker>(symbolTable.get());
}

void SemanticAnalyzer::analyze(std::shared_ptr<ASTNode> root)
{
    if (!root)
    {
        addError("Cannot analyze null AST");
        return;
    }

    std::cout << "\n=== Starting Semantic Analysis ===" << std::endl;

    // Reset state
    errors.clear();
    inFunction = false;
    currentFunction.reset();

    // PASS 1: Declaration collection
    std::cout << "\n[PASS 1] Collecting declarations..." << std::endl;
    collectDeclarations(root);

    if (hasErrors())
    {
        std::cout << "✗ Declaration collection failed with errors" << std::endl;
        return;
    }

    // PASS 2: Type checking and resolution
    std::cout << "\n[PASS 2] Type checking and resolution..." << std::endl;
    resolveAndTypeCheck(root);

    if (hasErrors())
    {
        std::cout << "✗ Type checking failed with errors" << std::endl;
        return;
    }

    // PASS 3: Initialization tracking
    std::cout << "\n[PASS 3] Initialization tracking..." << std::endl;
    trackInitialization(root);

    // Report results
    std::cout << "\n=== Semantic Analysis Complete ===" << std::endl;
    std::cout << "Total errors: " << errors.size() << std::endl;

    if (!hasErrors())
    {
        std::cout << "✓ All semantic checks passed!" << std::endl;
    }
    else
    {
        std::cout << "✗ Semantic analysis found errors:" << std::endl;
        for (const auto &error : errors)
        {
            std::cout << "  [LINE " << error.getLine() << "] " << error.what() << std::endl;
        }
    }
}

// ==================== PASS 1: Declaration Collection ====================

void SemanticAnalyzer::collectDeclarations(std::shared_ptr<ASTNode> node)
{
    if (!node)
        return;

    // Use dynamic_cast to identify node types
    if (auto funcDecl = std::dynamic_pointer_cast<FunctionDecl>(node))
    {
        visit(funcDecl);
    }
    else if (auto varDecl = std::dynamic_pointer_cast<VarDecl>(node))
    {
        visit(varDecl);
    }

    // Recursively process children
    // For simplicity, we'll handle common cases
    // In a real implementation, you'd have proper traversal
}

void SemanticAnalyzer::visit(std::shared_ptr<FunctionDecl> node)
{
    // Create function symbol
    DataType returnType = node->getReturnDataType();
    SourceLocation loc(node->location.line, node->location.column);

    auto funcSymbol = std::make_shared<Symbol>(
        node->getFunctionName(),
        SymbolType::FUNCTION,
        returnType,
        loc,
        node);

    // Insert into symbol table
    if (!symbolTable->insertSymbol(node->getFunctionName(), funcSymbol))
    {
        addError("Redeclaration of function: " + node->getFunctionName(),
                 node->location.line, node->location.column);
        return;
    }

    // Store symbol reference in AST node
    node->setSymbol(funcSymbol);

    std::cout << "  Declared function: " << node->getFunctionName()
              << " -> " << typeChecker->dataTypeToString(returnType) << std::endl;

    // Enter function scope
    symbolTable->enterScope(ScopeType::FUNCTION, node->getFunctionName());
    inFunction = true;
    currentFunction = funcSymbol;

    // Process parameters
    for (size_t i = 0; i < node->getParamCount(); ++i)
    {
        if (auto param = node->getParameter(i)) // param is VarDecl*
        {
            // Convert raw pointer to shared_ptr
            std::shared_ptr<VarDecl> paramShared(param, [](VarDecl *) {});
            visit(paramShared);
        }
    }

    // Process function body (declarations within)
    if (node->hasBody())
    {
        processFunctionBody(node->getBody());
    }

    // Exit function scope
    symbolTable->exitScope();
    inFunction = false;
    currentFunction.reset();
}

void SemanticAnalyzer::visit(std::shared_ptr<VarDecl> node)
{
    DataType varType = node->getDeclaredDataType();
    SourceLocation loc(node->location.line, node->location.column);

    auto varSymbol = std::make_shared<Symbol>(
        node->getVarName(),
        SymbolType::VARIABLE,
        varType,
        loc,
        node);

    // Check if variable has initializer
    if (node->hasInitializer())
    {
        varSymbol->setInitialized(true);
    }

    // Insert into symbol table
    if (!symbolTable->insertSymbol(node->getVarName(), varSymbol))
    {
        addError("Redeclaration of variable: " + node->getVarName(),
                 node->location.line, node->location.column);
        return;
    }

    // Store symbol reference in AST node
    node->setSymbol(varSymbol);

    std::cout << "  Declared variable: " << node->getVarName()
              << " -> " << typeChecker->dataTypeToString(varType) << std::endl;
}

void SemanticAnalyzer::processFunctionBody(CompoundStmt *body)
{
    if (!body)
        return;

    // Enter block scope for function body
    symbolTable->enterScope(ScopeType::BLOCK);

    // Process statements in body
    for (size_t i = 0; i < body->getStatementCount(); ++i)
    {
        if (auto stmt = body->getStatement(i))
        {
            // Look for variable declarations
            if (auto varDecl = dynamic_cast<VarDecl *>(stmt))
            {
                visit(std::shared_ptr<VarDecl>(varDecl, [](auto *) {}));
            }
        }
    }

    // Exit block scope
    symbolTable->exitScope();
}

// ==================== PASS 2: Type Checking & Resolution ====================

void SemanticAnalyzer::resolveAndTypeCheck(std::shared_ptr<ASTNode> node)
{
    if (!node)
        return;

    // This would be a full traversal of the AST
    // For now, we'll implement key visitor methods
}

// void SemanticAnalyzer::visit(std::shared_ptr<Assignment> node)
// {
//     // TODO: Implement assignment type checking
//     ASTVisitor::visit(node);
// }

void SemanticAnalyzer::visit(std::shared_ptr<IfStmt> node)
{
    std::cout << "  Analyzing if statement" << std::endl;

    // Check condition expression
    if (node->getCondition())
    {
        // Analyze condition
        DataType condType = typeChecker->getExpressionType(
            std::shared_ptr<Expr>(node->getCondition(), [](auto *) {}));

        // Condition should be convertible to boolean (int in C)
        if (!typeChecker->areCompatible(DataType::INT, condType, "if"))
        {
            std::stringstream ss;
            ss << "If condition must be boolean/int type, got: "
               << typeChecker->dataTypeToString(condType);
            addError(ss.str(), node->location.line, node->location.column);
        }
    }

    ASTVisitor::visit(node);
}

void SemanticAnalyzer::visit(std::shared_ptr<WhileStmt> node)
{
    std::cout << "  Analyzing while statement" << std::endl;

    // Check condition expression
    if (node->getCondition())
    {
        // Analyze condition
        DataType condType = typeChecker->getExpressionType(
            std::shared_ptr<Expr>(node->getCondition(), [](auto *) {}));

        // Condition should be convertible to boolean (int in C)
        if (!typeChecker->areCompatible(DataType::INT, condType, "while"))
        {
            std::stringstream ss;
            ss << "While condition must be boolean/int type, got: "
               << typeChecker->dataTypeToString(condType);
            addError(ss.str(), node->location.line, node->location.column);
        }
    }

    ASTVisitor::visit(node);
}

void SemanticAnalyzer::visit(std::shared_ptr<ReturnStmt> node)
{
    std::cout << "  Analyzing return statement" << std::endl;

    if (!inFunction || !currentFunction)
    {
        addError("Return statement outside of function",
                 node->location.line, node->location.column);
        return;
    }

    DataType expectedType = currentFunction->getDataType();

    if (node->hasValue())
    {
        // Get actual return expression type
        DataType actualType = typeChecker->getExpressionType(
            std::shared_ptr<Expr>(node->getValue(), [](auto *) {}));

        // Check type compatibility
        if (!typeChecker->areCompatible(expectedType, actualType, "return"))
        {
            std::stringstream ss;
            ss << "Return type mismatch. Expected: "
               << typeChecker->dataTypeToString(expectedType)
               << ", Got: " << typeChecker->dataTypeToString(actualType);
            addError(ss.str(), node->location.line, node->location.column);
        }
    }
    else if (expectedType != DataType::VOID)
    {
        // Non-void function returning nothing
        addError("Non-void function should return a value",
                 node->location.line, node->location.column);
    }

    ASTVisitor::visit(node);
}

void SemanticAnalyzer::visit(std::shared_ptr<BinaryExpr> node)
{
    std::cout << "  Analyzing binary expression" << std::endl;

    // Resolve left and right expressions
    ASTVisitor::visit(node);

    // Get types
    DataType leftType = typeChecker->getExpressionType(
        std::shared_ptr<Expr>(node->getLeft(), [](auto *) {}));
    DataType rightType = typeChecker->getExpressionType(
        std::shared_ptr<Expr>(node->getRight(), [](auto *) {}));

    // Convert TokenType to operator string
    std::string op = tokenTypeToOperatorString(node->getOperator());

    // Check compatibility
    if (!typeChecker->areCompatible(leftType, rightType, op))
    {
        std::stringstream ss;
        ss << "Type mismatch in binary expression: "
           << typeChecker->dataTypeToString(leftType)
           << " " << op << " "
           << typeChecker->dataTypeToString(rightType);
        addError(ss.str(), node->location.line, node->location.column);
    }

    // Set result type on node
    DataType resultType = typeChecker->getResultType(leftType, rightType, op);
    node->setDataType(resultType);

    std::cout << "    Result type: " << typeChecker->dataTypeToString(resultType) << std::endl;
}

void SemanticAnalyzer::visit(std::shared_ptr<CallExpr> node)
{
    std::cout << "  Analyzing function call: " << node->getFunctionName() << std::endl;

    // Look up function
    auto funcSymbol = symbolTable->lookup(node->getFunctionName());

    if (!funcSymbol)
    {
        addError("Undeclared function: " + node->getFunctionName(),
                 node->location.line, node->location.column);
        return;
    }

    // Store symbol reference
    node->setSymbol(funcSymbol);

    // Check function call
    if (!typeChecker->checkFunctionCall(node, funcSymbol))
    {
        addError("Invalid function call: " + node->getFunctionName(),
                 node->location.line, node->location.column);
    }

    // Set return type on call node
    DataType returnType = funcSymbol->getDataType();
    node->setDataType(returnType);

    std::cout << "    Return type: " << typeChecker->dataTypeToString(returnType) << std::endl;

    ASTVisitor::visit(node);
}

// Helper function to convert TokenType to operator string
std::string SemanticAnalyzer::tokenTypeToOperatorString(TokenType tokenType)
{
    // This is a simplified mapping - adjust based on your actual TokenType values
    switch (static_cast<int>(tokenType))
    {
    // Arithmetic
    case 1:
        return "+";
    case 2:
        return "-";
    case 3:
        return "*";
    case 4:
        return "/";
    case 5:
        return "%";

    // Comparison
    case 6:
        return "==";
    case 7:
        return "!=";
    case 8:
        return "<";
    case 9:
        return ">";
    case 10:
        return "<=";
    case 11:
        return ">=";

    // Assignment
    case 12:
        return "=";

    // Logical
    case 13:
        return "&&";
    case 14:
        return "||";

    // Bitwise
    case 15:
        return "&";
    case 16:
        return "|";
    case 17:
        return "^";
    case 18:
        return "<<";
    case 19:
        return ">>";

    default:
        return "?";
    }
}

// ==================== PASS 3: Initialization Tracking ====================

void SemanticAnalyzer::trackInitialization(std::shared_ptr<ASTNode> root)
{
    // Use InitializationTracker to analyze each function
    // For now, we'll do a simple traversal

    if (auto funcDecl = std::dynamic_pointer_cast<FunctionDecl>(root))
    {
        initTracker->analyzeFunction(funcDecl);

        // Collect errors from initTracker
        for (const auto &error : initTracker->getErrors())
        {
            addError(error, funcDecl->location.line, funcDecl->location.column);
        }
    }
}

// ==================== Utility Methods ====================

void SemanticAnalyzer::addError(const std::string &msg, int line, int col)
{
    errors.emplace_back(msg, line, col);
}

DataType SemanticAnalyzer::getTypeFromString(const std::string &typeStr)
{
    return typeChecker->typeFromString(typeStr);
}

void SemanticAnalyzer::processVariableDeclaration(std::shared_ptr<VarDecl> node)
{
    // Already implemented in visit(VarDecl)
}