#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "semantic/SymbolTable.hpp"
#include "semantic/Symbol.hpp"
#include "utils/SourceLocation.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "cfg/CFGBuilder.hpp"
#include "cfg/CFG.hpp"
#include "utils/FunctionExtractor.hpp" // ADD THIS

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file.c>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }

    // Read entire file
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    std::cout << "=== Phase A: Tokenization ===" << std::endl;
    Tokenizer tokenizer(source);
    auto tokens = tokenizer.tokenize();
    std::cout << "Tokens: " << tokens.size() << " tokens generated\n";

    std::cout << "\n=== Phase B: Parsing ===" << std::endl;
    Parser parser(tokens);
    auto ast = parser.parse();
    // After parsing, add:
    std::cout << "\n=== AST Root Type ===" << std::endl;
    if (ast)
    {
        std::cout << "AST Root Type: ";
        if (dynamic_cast<FunctionDecl *>(ast.get()))
            std::cout << "FunctionDecl";
        else if (dynamic_cast<CompoundStmt *>(ast.get()))
            std::cout << "CompoundStmt";
        else if (dynamic_cast<IfStmt *>(ast.get()))
            std::cout << "IfStmt";
        else
            std::cout << "Unknown";
        std::cout << std::endl;
    }
    if (!ast)
    {
        std::cout << "\n✗ Parsing failed!\n";
        return 1;
    }

    std::cout << "\n=== AST Structure ===" << std::endl;
    ast->print();

    std::cout << "\n=== Phase C: Semantic Analysis ===" << std::endl;
    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(std::move(ast));

    if (semanticAnalyzer.hasErrors())
    {
        std::cout << "\n✗ Semantic analysis failed!" << std::endl;
        return 1;
    }

    std::cout << "\n✓ Semantic analysis passed!" << std::endl;

    // Optional: Print symbol table
    std::cout << "\n=== Symbol Table ===" << std::endl;
    semanticAnalyzer.getSymbolTable()->print();

    // ========== PHASE D: COMPLETE CFG BUILDING ==========
    std::cout << "\n=== Phase D: Control Flow Graph Construction ===" << std::endl;

    // Get functions from the AST
    auto functions = FunctionExtractor::extractFunctions(semanticAnalyzer.getAST());

    std::cout << "Found " << functions.size() << " function(s)" << std::endl;

    if (!functions.empty())
    {
        // Create CFG Builder
        CFGBuilder cfgBuilder(semanticAnalyzer.getSymbolTable());

        for (const auto &function : functions)
        {
            std::cout << "\n--- Building CFG for function: "
                      << function->getFunctionName() << " ---" << std::endl;

            try
            {
                // ACTUALLY BUILD THE CFG
                std::shared_ptr<CFG> cfg = cfgBuilder.buildCFG(function);

                if (cfg)
                {
                    std::cout << "✓ Successfully built CFG" << std::endl;

                    // Print detailed CFG information
                    std::cout << "  CFG has " << cfg->getBlocks().size() << " blocks" << std::endl;
                    std::cout << "  CFG has " << cfg->getEdges().size() << " edges" << std::endl;

                    // Print CFG structure
                    cfg->print();

                    // Verify CFG
                    if (cfg->verify())
                    {
                        std::cout << "  ✓ CFG verification passed" << std::endl;
                    }
                    else
                    {
                        std::cout << "  ⚠ CFG verification warnings" << std::endl;
                    }
                }
                else
                {
                    std::cout << "✗ Failed to build CFG" << std::endl;
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "✗ Error building CFG: " << e.what() << std::endl;
            }
        }

        std::cout << "\n=== Phase D Complete ===" << std::endl;
        std::cout << "Built CFGs for " << functions.size() << " function(s)" << std::endl;
    }
    else
    {
        std::cout << "No functions found in the AST" << std::endl;
    }

    return 0;
}