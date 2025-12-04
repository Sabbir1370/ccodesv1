cmake_minimum_required(VERSION 3.10)
project(C_Code_Analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== PHASE D - CFG FILES ====================
set(CFG_SOURCES
    src/cfg/BasicBlock.cpp
    src/cfg/Edge.cpp
    src/cfg/CFG.cpp
    src/cfg/CFGBuilder.cpp
)

set(CFG_HEADERS
    src/cfg/BasicBlock.hpp
    src/cfg/Edge.hpp
    src/cfg/CFG.hpp
    src/cfg/CFGBuilder.hpp
)

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

# ==================== UTILS FILES ====================
set(UTILS_SOURCES
    src/utils/Logger.hpp  # Header only
    src/utils/SourceLocation.hpp  # Header only
    src/utils/FunctionExtractor.cpp  # ADD THIS
)

# ==================== MAIN PROGRAM ====================
add_executable(c-analyzer 
    src/main.cpp
    # Phase B - Parser
    src/parser/tokenizer.cpp
    src/parser/parser.cpp
    # AST files
    src/ast/ast_nodes.cpp
    src/ast/ast_visitor.cpp
    
    # Phase C - Semantic
    ${SEMANTIC_SOURCES}
    
    # Phase D - CFG (NEW)
    ${CFG_SOURCES}
    ${UTILS_SOURCES}
)

target_include_directories(c-analyzer PRIVATE
    src           # Main include directory
    src/parser    # Parser headers
    src/ast       # AST headers
    src/semantic  # Semantic headers
    src/utils     # Utility headers
    src/cfg       # NEW: CFG headers
)

target_compile_options(c-analyzer PRIVATE 
    -Wall 
    -Wextra 
    -O2
)

# Optional: If you want to keep your test_parser (but not required)
if(FALSE)  # Set to TRUE if you want to keep tests
    add_executable(test_parser
        test/unit/test_tokenizer.cpp
        test/unit/test_parser.cpp
        src/parser/tokenizer.cpp
        src/parser/parser.cpp
        src/ast/ast_nodes.cpp
        src/ast/ast_visitor.cpp
        ${SEMANTIC_SOURCES}
        ${CFG_SOURCES}
    )

    target_include_directories(test_parser PRIVATE
        src
        src/parser
        src/ast
        src/semantic
        src/utils
        src/cfg
    )
    target_compile_options(test_parser PRIVATE -Wall -Wextra -O0 -g)
endif()