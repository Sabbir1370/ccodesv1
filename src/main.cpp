#include <iostream>
#include <fstream>
#include <string>
#include "parser/tokenizer.h"
#include "parser/parser.h" // Add this

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

    // Debug: Print tokens
    std::cout << "=== Tokens ===" << tokens.size() << " tokens:\n";
    for (const auto &token : tokens)
    {
        std::cout << "Line " << token.line << ":" << token.column
                  << " Type=" << static_cast<int>(token.type)
                  << " Lexeme='" << token.lexeme << "'\n";
    }

    // Parse
    std::cout << "\n=== AST ===\n";
    Parser parser(tokens);
    auto ast = parser.parse();

    if (ast)
    {
        ast->print();
        std::cout << "\nParsing successful!\n";
    }
    else
    {
        std::cout << "\nParsing failed!\n";
    }

    return 0;
}