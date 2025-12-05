// src/detectors/detectors/FormatStringInspector.cpp
#include "FormatStringInspector.hpp"
#include <iostream>
#include <sstream>
#include <functional> // Added for std::function
namespace detectors
{

    FormatStringInspector::FormatStringInspector()
        : VulnerabilityDetector("FMT001",
                                "Format String Vulnerability Detection")
    {
        // Default configuration
        config_.severity_override = Severity::HIGH;
        config_.risk_weight = 2;
    }
    std::vector<Finding> FormatStringInspector::analyze(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        std::vector<Finding> findings;

        if (!ast || !config_.enabled)
        {
            return findings;
        }

        // Recursive analysis function
        std::function<void(ASTNode *)> analyzeNode = [&](ASTNode *node)
        {
            if (!node)
                return;

            // Handle CallExpr
            if (auto callExpr = dynamic_cast<CallExpr *>(node))
            {
                analyzeCallExpression(callExpr, findings, symtab);
                return;
            }

            // Handle FunctionDecl - GO INTO FUNCTION BODIES!
            if (auto funcDecl = dynamic_cast<FunctionDecl *>(node))
            {
                if (funcDecl->hasBody())
                {
                    analyzeNode(funcDecl->getBody());
                }
                return;
            }

            // Handle other node types and recurse
            if (auto compound = dynamic_cast<CompoundStmt *>(node))
            {
                for (size_t i = 0; i < compound->getStatementCount(); i++)
                {
                    analyzeNode(compound->getStatement(i));
                }
                return;
            }
            else if (auto exprStmt = dynamic_cast<ExprStmt *>(node))
            {
                analyzeNode(exprStmt->getExpression());
                return;
            }
            else if (auto ifStmt = dynamic_cast<IfStmt *>(node))
            {
                analyzeNode(ifStmt->getCondition());
                analyzeNode(ifStmt->getThenBranch());
                if (ifStmt->hasElseBranch())
                {
                    analyzeNode(ifStmt->getElseBranch());
                }
                return;
            }
            else if (auto whileStmt = dynamic_cast<WhileStmt *>(node))
            {
                analyzeNode(whileStmt->getCondition());
                analyzeNode(whileStmt->getBody());
                return;
            }
            else if (auto returnStmt = dynamic_cast<ReturnStmt *>(node))
            {
                if (returnStmt->hasValue())
                {
                    analyzeNode(returnStmt->getValue());
                }
                return;
            }
            else if (auto binaryExpr = dynamic_cast<BinaryExpr *>(node))
            {
                analyzeNode(binaryExpr->getLeft());
                analyzeNode(binaryExpr->getRight());
                return;
            }
            else if (auto unaryExpr = dynamic_cast<UnaryExpr *>(node))
            {
                analyzeNode(unaryExpr->getOperand());
                return;
            }
            else if (auto varDecl = dynamic_cast<VarDecl *>(node))
            {
                if (varDecl->hasInitializer())
                {
                    analyzeNode(varDecl->getInitializer());
                }
                return;
            }
            // Add more node types as needed
        };

        // Start analysis
        analyzeNode(ast.get());

        return findings;
    }
    bool FormatStringInspector::isFormatStringFunction(const std::string &funcName)
    {
        return formatFunctions.find(funcName) != formatFunctions.end();
    }

    void FormatStringInspector::analyzeCallExpression(
        CallExpr *callExpr,
        std::vector<Finding> &findings,
        SymbolTable *symtab)
    {

        if (!callExpr)
            return;

        std::string funcName = callExpr->getFunctionName();

        if (!isFormatStringFunction(funcName))
        {
            return;
        }
        // std::cout << "[FMT001] Checking call to: " << funcName << std::endl;

        // Get arguments
        std::vector<Expr *> args;
        for (size_t i = 0; i < callExpr->getArgCount(); i++)
        {
            if (auto arg = callExpr->getArgument(i))
            {
                args.push_back(arg);
            }
        }

        // Need at least format string argument
        if (args.empty())
        {
            // Convert SourceLoc to SourceLocation
            SourceLocation location(callExpr->location.line, callExpr->location.column);

            auto finding = createBaseFinding(
                location,
                Severity::HIGH,
                "Format string function called without arguments");
            finding.function_name = funcName;
            findings.push_back(finding);
            return;
        }

        // First argument should be format string
        Expr *formatArg = args[0];

        // Collect other arguments
        std::vector<Expr *> otherArgs;
        for (size_t i = 1; i < args.size(); i++)
        {
            otherArgs.push_back(args[i]);
        }

        checkFormatString(callExpr, formatArg, otherArgs, findings);
    }

    void FormatStringInspector::checkFormatString(
        CallExpr *callExpr,
        Expr *formatArg,
        const std::vector<Expr *> &otherArgs,
        std::vector<Finding> &findings)
    {
        // std::cout << "[FMT001]   Checking format string..." << std::endl;

        // Check if format argument is a string literal
        if (auto literal = dynamic_cast<LiteralExpr *>(formatArg))
        {
            // std::cout << "[FMT001]   Format arg is literal, type value: "
            //           << static_cast<int>(literal->literalType) << std::endl;

            // // Debug: Print all possible token types
            // std::cout << "[FMT001-DEBUG] Checking if it's a string literal..." << std::endl;

            // Try to identify string literal by value (fallback)
            std::string value = literal->value;
            bool looksLikeString = (!value.empty() &&
                                    ((value.front() == '"' && value.back() == '"') ||
                                     (value.front() == '\'' && value.back() == '\'')));

            // std::cout << "[FMT001-DEBUG] Value: " << value
            //           << ", looksLikeString: " << looksLikeString << std::endl;

            if (looksLikeString || literal->literalType == TokenType::LITERAL_STRING)
            {
                // std::cout << "[FMT001]   Found string literal: " << literal->value << std::endl;

                // Remove quotes for analysis
                std::string formatStr = value;
                if (formatStr.size() >= 2 &&
                    ((formatStr.front() == '"' && formatStr.back() == '"') ||
                     (formatStr.front() == '\'' && formatStr.back() == '\'')))
                {
                    formatStr = formatStr.substr(1, formatStr.size() - 2);
                }

                // std::cout << "[FMT001]   Format string (no quotes): " << formatStr << std::endl;

                int specifierCount = countFormatSpecifiers(formatStr);
                // std::cout << "[FMT001]   Found " << specifierCount
                //   << " format specifiers, " << otherArgs.size()
                //   << " arguments provided" << std::endl;

                // Check argument count mismatch
                if (specifierCount != otherArgs.size())
                {
                    std::stringstream desc;
                    desc << "Format string expects " << specifierCount
                         << " arguments but " << otherArgs.size() << " provided";

                    // Convert SourceLoc to SourceLocation
                    SourceLocation location(callExpr->location.line, callExpr->location.column);

                    auto finding = createBaseFinding(
                        location,
                        Severity::HIGH,
                        desc.str());
                    finding.function_name = callExpr->getFunctionName();
                    findings.push_back(finding);

                    // std::cout << "[FMT001]   Added finding: " << desc.str() << std::endl;
                }
                else
                {
                    // std::cout << "[FMT001]   Argument count matches, OK" << std::endl;
                }
            }
            else
            {
                std::cout << "[FMT001]   Not a string literal, skipping" << std::endl;
            }
        }
        else
        {
            // std::cout << "[FMT001]   Format arg is NOT a literal!" << std::endl;
            // Format string is not a literal - potential vulnerability!
            SourceLocation location(callExpr->location.line, callExpr->location.column);

            auto finding = createBaseFinding(
                location,
                Severity::CRITICAL,
                "Non-constant format string used - potential format string vulnerability");
            finding.function_name = callExpr->getFunctionName();
            findings.push_back(finding);

            // std::cout << "[FMT001]   Added CRITICAL finding for variable format string" << std::endl;
        }
    }
    int FormatStringInspector::countFormatSpecifiers(const std::string &formatStr)
    {
        int count = 0;
        bool inPercent = false;

        for (size_t i = 0; i < formatStr.length(); i++)
        {
            if (formatStr[i] == '%')
            {
                if (inPercent)
                {
                    // %% is escaped percent
                    inPercent = false;
                }
                else
                {
                    inPercent = true;
                }
            }
            else if (inPercent)
            {
                // Check for valid specifier
                std::string specifier = "%";
                specifier += formatStr[i];

                // Handle length modifiers (like %ld, %lf)
                if ((formatStr[i] == 'l' || formatStr[i] == 'h' ||
                     formatStr[i] == 'L') &&
                    i + 1 < formatStr.length())
                {
                    specifier += formatStr[i + 1];
                    i++; // Skip next character
                }

                // Count valid specifiers
                bool isValid = false;
                for (const auto &spec : formatSpecifiers)
                {
                    if (specifier.find(spec) == 0)
                    {
                        isValid = true;
                        break;
                    }
                }

                if (isValid)
                {
                    count++;
                }
                inPercent = false;
            }
        }

        return count;
    }

    std::string FormatStringInspector::getExpectedArgType(const std::string &specifier)
    {
        // Simple type mapping
        if (specifier == "%d" || specifier == "%i")
            return "int";
        if (specifier == "%u")
            return "unsigned int";
        if (specifier == "%f" || specifier == "%lf")
            return "double";
        if (specifier == "%c")
            return "char";
        if (specifier == "%s")
            return "char*";
        if (specifier == "%p")
            return "void*";
        if (specifier == "%x" || specifier == "%o")
            return "unsigned int";
        if (specifier == "%n")
            return "int*";
        return "unknown";
    }

} // namespace detectors