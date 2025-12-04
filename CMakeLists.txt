cmake_minimum_required(VERSION 3.10)
project(C_Code_Analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== SEMANTIC FILES ====================
set(SEMANTIC_SOURCES
    src/semantic/Symbol.cpp
    src/semantic/Scope.cpp
    src/semantic/SymbolTable.cpp
    src/semantic/TypeChecker.cpp
    src/semantic/InitializationTracker.cpp
    src/semantic/SemanticAnalyzer.cpp
)

set(SEMANTIC_HEADERS
    src/semantic/Symbol.hpp
    src/semantic/Scope.hpp
    src/semantic/SymbolTable.hpp
    src/semantic/TypeChecker.hpp
    src/semantic/InitializationTracker.hpp
    src/semantic/SemanticAnalyzer.hpp
)

# ==================== MAIN PROGRAM ====================
add_executable(c-analyzer 
    src/main.cpp
    # Phase B - Parser
    src/parser/tokenizer.cpp
    src/parser/parser.cpp
    # AST files
    src/ast/ast_nodes.cpp
    src/ast/ast_visitor.cpp      # <-- ADD THIS LINE
    
    # Phase C - Semantic
    ${SEMANTIC_SOURCES}
)

target_include_directories(c-analyzer PRIVATE
    src          # Add this to find headers in src/parser, src/ast
    src/parser
    src/ast
    src/semantic
    src/utils
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
    src/ast/ast_visitor.cpp      # <-- ADD THIS LINE
    ${SEMANTIC_SOURCES}
)

target_include_directories(test_parser PRIVATE
    src
    src/parser
    src/ast
    src/semantic
    src/utils
)
target_compile_options(test_parser PRIVATE -Wall -Wextra -O0 -g)