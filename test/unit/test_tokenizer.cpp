#include "tokenizer.h"
#include <cassert>
#include <iostream>

void test_basic()
{
    std::string code = "int main() { return 0; }";
    Tokenizer tokenizer(code);
    auto tokens = tokenizer.tokenize();

    assert(tokens.size() > 0);
    assert(tokens.back().type == TokenType::END_OF_FILE);
    std::cout << "✓ Basic test passed\n";
}

void test_comments()
{
    std::string code = "// comment\nint x; /* multiline */";
    Tokenizer tokenizer(code);
    auto tokens = tokenizer.tokenize();

    // Should have: int, x, ;, EOF
    assert(tokens.size() == 4);
    std::cout << "✓ Comments test passed\n";
}

void test_keywords()
{
    std::string code = "int if while return";
    Tokenizer tokenizer(code);
    auto tokens = tokenizer.tokenize();

    // Check first token is KEYWORD_INT
    assert(tokens[0].type == TokenType::KEYWORD_INT);
    assert(tokens[0].lexeme == "int");
    std::cout << "✓ Keywords test passed\n";
}

int main()
{
    test_basic();
    test_comments();
    test_keywords();
    std::cout << "All tokenizer tests passed!\n";
    return 0;
}