cmake_minimum_required(VERSION 3.10)
project(C_Code_Analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== MAIN PROGRAM ====================
add_executable(c-analyzer 
    src/main.cpp
    # Phase B - Parser
    src/parser/tokenizer.cpp
    src/parser/parser.cpp
    # src/parser/ast_builder.cpp  # Remove or create this file
    src/ast/ast_nodes.cpp
    # src/ast/ast_visitor.cpp     # Remove for Phase B
)

target_include_directories(c-analyzer PRIVATE
    src          # Add this to find headers in src/parser, src/ast
    src/parser
    src/ast
)

target_compile_options(c-analyzer PRIVATE -Wall -Wextra -O2)

# ==================== TESTS ====================
# Test framework
add_executable(test_harness test/unit/test_harness.cpp)
target_compile_options(test_harness PRIVATE -Wall -Wextra -O0 -g)

# Parser tests
add_executable(test_parser
    test/unit/test_tokenizer.cpp
    test/unit/test_parser.cpp
    src/parser/tokenizer.cpp
    src/parser/parser.cpp
    src/ast/ast_nodes.cpp
)

target_include_directories(test_parser PRIVATE
    src
    src/parser
    src/ast
)
target_compile_options(test_parser PRIVATE -Wall -Wextra -O0 -g)