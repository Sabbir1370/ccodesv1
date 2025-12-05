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

# ==================== SEMANTIC FILES ====================
set(SEMANTIC_SOURCES
    src/semantic/Symbol.cpp
    src/semantic/Scope.cpp
    src/semantic/SymbolTable.cpp
    src/semantic/TypeChecker.cpp
    src/semantic/InitializationTracker.cpp
    src/semantic/SemanticAnalyzer.cpp
)

# ==================== UTILS FILES ====================
set(UTILS_SOURCES
    src/utils/FunctionExtractor.cpp  # Only .cpp files here
)

# ==================== DETECTOR FRAMEWORK ====================
set(DETECTOR_SOURCES
    src/detectors/Finding.cpp
    src/detectors/VulnerabilityDetector.cpp
    src/detectors/DetectorManager.cpp
)

# ==================== INDIVIDUAL DETECTORS ====================
# Phase E: SecureMemTracker
set(DETECTORS_PHASE_E
    src/detectors/detectors/SecureMemTracker.cpp
)

# Phase F: Taint Analysis
set(DETECTORS_PHASE_F
    src/detectors/detectors/FunctionContext.cpp
    src/detectors/detectors/TaintFlowDetector.cpp
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
    
    # Phase D - CFG
    ${CFG_SOURCES}
    
    # Utils
    ${UTILS_SOURCES}
    
    # Detector Framework
    ${DETECTOR_SOURCES}
    
    # Phase E Detector
    ${DETECTORS_PHASE_E}
    
    # Phase F Detector
    ${DETECTORS_PHASE_F}
)

# ==================== INCLUDE DIRECTORIES ====================
target_include_directories(c-analyzer PRIVATE
    src
    src/ast
    src/parser
    src/semantic
    src/cfg
    src/utils
    src/detectors
    src/detectors/detectors  # For TaintFlowDetector.hpp, FunctionContext.hpp
)

# ==================== COMPILER OPTIONS ====================
target_compile_options(c-analyzer PRIVATE 
    -Wall 
    -Wextra 
    -O2
    -std=c++17
)