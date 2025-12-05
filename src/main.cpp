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
#include "utils/FunctionExtractor.hpp"

// === PHASE E: DETECTOR FRAMEWORK INCLUDES ===
#include "detectors/DetectorManager.hpp"
#include "detectors/detectors/SecureMemTracker.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file.c>\n";
        std::cerr << "Options:\n";
        std::cerr << "  --enable-all    Enable all security detectors\n";
        std::cerr << "  --disable-all   Disable all security detectors\n";
        std::cerr << "  --list-detectors List all available detectors\n";
        return 1;
    }

    std::string filename = argv[1];

    // Check command line options
    bool enable_all = false;
    bool disable_all = false;
    bool list_detectors = false;

    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--enable-all")
            enable_all = true;
        else if (arg == "--disable-all")
            disable_all = true;
        else if (arg == "--list-detectors")
            list_detectors = true;
    }

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

    std::cout << "=== C Code Security Analyzer (Phase E) ===" << std::endl;
    std::cout << "Analyzing: " << filename << std::endl;

    std::cout << "\n=== Phase A: Tokenization ===" << std::endl;
    Tokenizer tokenizer(source);
    auto tokens = tokenizer.tokenize();
    std::cout << "✓ Generated " << tokens.size() << " tokens\n";

    std::cout << "\n=== Phase B: Parsing ===" << std::endl;
    Parser parser(tokens);
    auto ast = parser.parse();

    if (!ast)
    {
        std::cout << "\n✗ Parsing failed!\n";
        return 1;
    }

    std::cout << "✓ Parsing successful\n";

    std::cout << "\n=== Phase C: Semantic Analysis ===" << std::endl;
    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(std::move(ast));

    if (semanticAnalyzer.hasErrors())
    {
        std::cout << "\n✗ Semantic analysis failed!" << std::endl;
        return 1;
    }

    std::cout << "✓ Semantic analysis passed!" << std::endl;

    // ========== PHASE D: CFG CONSTRUCTION ==========
    std::cout << "\n=== Phase D: Control Flow Graph Construction ===" << std::endl;

    // Get functions from the AST
    auto functions = FunctionExtractor::extractFunctions(semanticAnalyzer.getAST());
    std::cout << "Found " << functions.size() << " function(s)" << std::endl;

    std::vector<std::shared_ptr<CFG>> all_cfgs; // Store all CFGs for Phase E

    if (!functions.empty())
    {
        // Create CFG Builder
        CFGBuilder cfgBuilder(semanticAnalyzer.getSymbolTable());

        for (const auto &function : functions)
        {
            try
            {
                std::shared_ptr<CFG> cfg = cfgBuilder.buildCFG(function);
                if (cfg)
                {
                    all_cfgs.push_back(cfg);
                }
            }
            catch (const std::exception &e)
            {
                // Silent fail for now, or log if needed
            }
        }
    }

    // ========== PHASE E: DETECTOR FRAMEWORK ==========
    std::cout << "\n=== Phase E: Security Vulnerability Detection ===" << std::endl;

    // Create detector manager
    detectors::DetectorManager detectorManager;

    // Register available detectors
    detectorManager.registerDetector(std::make_unique<detectors::SecureMemTracker>());
    // Add more detectors here as they are implemented

    std::cout << "Available detectors: " << detectorManager.getDetectorCount() << std::endl;

    // List detectors if requested
    if (list_detectors)
    {
        std::cout << "\n=== Detector List ===" << std::endl;
        for (const auto &detector : detectorManager.getDetectors())
        {
            std::cout << "  " << detector->getName() << ": " << detector->getDescription();
            std::cout << " [" << (detector->isEnabled() ? "ENABLED" : "DISABLED") << "]" << std::endl;
        }
    }

    // Configure detectors based on command line options
    if (enable_all)
    {
        for (const auto &detector : detectorManager.getDetectors())
        {
            detector->setEnabled(true);
        }
        std::cout << "All detectors enabled" << std::endl;
    }
    else if (disable_all)
    {
        for (const auto &detector : detectorManager.getDetectors())
        {
            detector->setEnabled(false);
        }
        std::cout << "All detectors disabled" << std::endl;
    }

    // Run security analysis
    std::cout << "\nRunning security analysis..." << std::endl;

    // Get AST and symbol table from semantic analyzer
    std::shared_ptr<ASTNode> ast_for_detectors = semanticAnalyzer.getAST();
    SymbolTable *symtab = semanticAnalyzer.getSymbolTable();

    auto findings = detectorManager.runEnabledDetectors(ast_for_detectors, symtab, all_cfgs);

    // Report results
    std::cout << "\n=== SECURITY ANALYSIS RESULTS ===" << std::endl;
    if (findings.empty())
    {
        std::cout << "✓ No security vulnerabilities detected." << std::endl;
        std::cout << "✓ All enabled detectors passed." << std::endl;
    }
    else
    {
        std::cout << "✗ Found " << findings.size() << " potential security issue(s):" << std::endl;
        std::cout << "==========================================" << std::endl;

        // Group findings by severity
        int critical = 0, high = 0, medium = 0, low = 0, info = 0;

        for (const auto &finding : findings)
        {
            switch (finding.severity)
            {
            case detectors::Severity::CRITICAL:
                critical++;
                break;
            case detectors::Severity::HIGH:
                high++;
                break;
            case detectors::Severity::MEDIUM:
                medium++;
                break;
            case detectors::Severity::LOW:
                low++;
                break;
            case detectors::Severity::INFO:
                info++;
                break;
            }
        }

        std::cout << "Severity breakdown:" << std::endl;
        if (critical > 0)
            std::cout << "  CRITICAL: " << critical << std::endl;
        if (high > 0)
            std::cout << "  HIGH: " << high << std::endl;
        if (medium > 0)
            std::cout << "  MEDIUM: " << medium << std::endl;
        if (low > 0)
            std::cout << "  LOW: " << low << std::endl;
        if (info > 0)
            std::cout << "  INFO: " << info << std::endl;

        std::cout << "\nDetailed findings:" << std::endl;
        std::cout << "==========================================" << std::endl;

        for (size_t i = 0; i < findings.size(); i++)
        {
            std::cout << "\nFinding #" << (i + 1) << ":\n";
            std::cout << findings[i].toString() << std::endl;
        }
    }

    // Summary
    std::cout << "\n=== ANALYSIS SUMMARY ===" << std::endl;
    std::cout << "Phases completed: A (Tokenization), B (Parsing), C (Semantic),";
    std::cout << " D (CFG), E (Detection)" << std::endl;
    std::cout << "Functions analyzed: " << functions.size() << std::endl;
    std::cout << "CFGs built: " << all_cfgs.size() << std::endl;
    std::cout << "Detectors run: " << detectorManager.getDetectorCount() << std::endl;
    std::cout << "Security issues found: " << findings.size() << std::endl;

    // Exit code based on findings
    if (!findings.empty())
    {
        std::cout << "\n⚠ Security vulnerabilities detected. Review recommended." << std::endl;
        return 2; // Exit code 2 for security issues found
    }

    std::cout << "\n✓ Analysis completed successfully." << std::endl;
    return 0;
}