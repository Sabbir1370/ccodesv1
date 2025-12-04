#include <iostream>
#include <fstream>
#include <string>
#include "parser/tokenizer.h"
#include "parser/parser.h" // Add this
#include "semantic/SymbolTable.hpp"
#include "semantic/Symbol.hpp"
#include "utils/SourceLocation.hpp"
#include "semantic/SemanticAnalyzer.hpp"

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

    // Tokenize
    Tokenizer tokenizer(source);
    auto tokens = tokenizer.tokenize();

    // Parse
    Parser parser(tokens);
    auto ast = parser.parse();

    if (ast)
    {
        std::cout << "\n=== AST ===" << std::endl;
        ast->print();

        // Run semantic analysis
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyze(std::move(ast));
        if (semanticAnalyzer.hasErrors())
        {
            std::cout << "\n✗ Semantic analysis failed!" << std::endl;
            return 1;
        }
        else
        {
            std::cout << "\n✓ Semantic analysis passed!" << std::endl;

            // Optional: Print symbol table
            std::cout << "\n=== Symbol Table ===" << std::endl;
            semanticAnalyzer.getSymbolTable()->print();
        }
    }
    else
    {
        std::cout << "\nParsing failed!\n";
        return 1;
    }

    return 0;
}