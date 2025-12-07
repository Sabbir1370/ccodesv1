#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <iomanip>
#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "semantic/SymbolTable.hpp"
#include "semantic/Symbol.hpp"
#include "utils/SourceLocation.hpp"
#include "semantic/SemanticAnalyzer.hpp"
#include "cfg/CFGBuilder.hpp"
#include "cfg/CFG.hpp"
#include "utils/FunctionExtractor.hpp"
#include "detectors/DetectorManager.hpp"
#include "detectors/detectors/SecureMemTracker.hpp"
#include "detectors/detectors/TaintFlowDetector.hpp"
#include "detectors/detectors/FormatStringInspector.hpp"
#include "detectors/detectors/UseBeforeInitDetector.hpp"
#include "detectors/detectors/SimpleBufferDetector.hpp"

// Risk Assessment headers (Phase I)
#include "risk/RiskAssessmentEngine.hpp"
// #include "risk/ComplianceChecker.hpp"
// #include "risk/RiskMetrics.hpp"
// #include "risk/RiskScoreCalculator.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file.c> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --enable-all          Enable all security detectors\n";
        std::cerr << "  --disable-all         Disable all security detectors\n";
        std::cerr << "  --list-detectors      List all available detectors\n";
        std::cerr << "  --policy <file>       Policy configuration file\n";
        std::cerr << "  --risk-report         Generate risk assessment report\n";
        std::cerr << "  --compliance          Check compliance standards\n";
        std::cerr << "  --output <file>       Output report file\n";
        std::cerr << "  --no-risk             Skip risk assessment (default)\n";
        return 1;
    }

    std::string filename = argv[1];

    // Check command line options
    bool enable_all = false;
    bool disable_all = false;
    bool list_detectors = false;
    bool generate_risk_report = false;
    bool check_compliance = false;
    bool skip_risk = false;
    std::string policy_file = "/home/zer0/ccodesv1/config/policy.json";
    std::string output_file = "";

    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--enable-all")
            enable_all = true;
        else if (arg == "--disable-all")
            disable_all = true;
        else if (arg == "--list-detectors")
            list_detectors = true;
        else if (arg == "--risk-report")
            generate_risk_report = true;
        else if (arg == "--compliance")
            check_compliance = true;
        else if (arg == "--no-risk")
            skip_risk = true;
        else if (arg == "--policy" && i + 1 < argc)
            policy_file = argv[++i];
        else if (arg == "--output" && i + 1 < argc)
            output_file = argv[++i];
    }

    // Default: if neither risk-report nor no-risk specified, ask user
    if (!generate_risk_report && !skip_risk)
    {
        std::cout << "Run risk assessment? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        if (response == "y" || response == "Y")
        {
            generate_risk_report = true;
            // Also ask about compliance
            std::cout << "Check compliance standards? (y/n): ";
            std::getline(std::cin, response);
            if (response == "y" || response == "Y")
            {
                check_compliance = true;
            }
        }
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

    std::cout << "=== C Code Security Analyzer ===" << std::endl;
    std::cout << "Analyzing: " << filename << std::endl;
    if (generate_risk_report)
        std::cout << "Risk Assessment: ENABLED" << std::endl;
    if (check_compliance)
        std::cout << "Compliance Check: ENABLED" << std::endl;

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

    // Load the policy
    std::cout << "Loading policy from: " << policy_file << std::endl;
    bool policy_loaded = detectorManager.loadPolicy(policy_file);

    if (!policy_loaded)
    {
        std::cout << "Warning: Could not load policy file. Using default detector settings." << std::endl;
    }

    std::cout << "\nDetector Status:" << std::endl;
    for (const auto &detector : detectorManager.getDetectors())
    {
        std::cout << "  - " << detector->getName()
                  << ": " << (detector->isEnabled() ? "ENABLED" : "DISABLED")
                  << std::endl;
    }

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

    // ========== PHASE I: RISK ASSESSMENT ==========
    if (generate_risk_report && !findings.empty())
    {
        std::cout << "\n=== Phase I: Risk Assessment & Compliance ===" << std::endl;

        try
        {
            // Create risk assessment engine
            auto risk_calculator = std::make_unique<risk::DefaultRiskScoreCalculator>();
            auto compliance_checker = std::make_unique<risk::DefaultComplianceChecker>();

            risk::RiskAssessmentEngine risk_engine(
                std::move(risk_calculator),
                std::move(compliance_checker));

            // Load detector weights from policy if available
            // Note: You'll need to add a method to DetectorManager to get weights
            // For now, we'll use default weights
            std::map<std::string, double> detector_weights = {
                {"MEM001", 1.5},   // SecureMemTracker
                {"TAINT001", 2.0}, // TaintFlowDetector
                {"FMT001", 1.0},   // FormatStringInspector
                {"INIT001", 1.0},  // UseBeforeInitDetector
                {"BUF001", 0.8}    // SimpleBufferDetector
            };
            risk_engine.setDetectorWeights(detector_weights);

            // Configure compliance standards to check
            std::vector<std::string> required_standards;
            if (check_compliance)
            {
                required_standards = {"CERT-C", "OWASP", "CWE"};
            }

            // Perform risk assessment
            auto risk_results = risk_engine.assessRisk(
                findings,
                required_standards,
                detectors::Severity::LOW // Include even low severity for risk assessment
            );

            // Output results
            if (!output_file.empty())
            {
                std::ofstream out_file(output_file);
                if (out_file.is_open())
                {
                    std::string report = risk_engine.generateRiskReport(risk_results);
                    out_file << report;
                    out_file.close();
                    std::cout << "✓ Risk report written to: " << output_file << std::endl;
                }
                else
                {
                    std::cerr << "✗ Could not open output file: " << output_file << std::endl;
                }
            }
            else
            {
                // Console output - Executive Summary
                std::cout << "\n"
                          << risk_engine.generateExecutiveSummary(risk_results) << "\n";

                // Optionally show full report in console
                if (findings.size() < 20)
                { // Only show full report for small numbers of findings
                    std::cout << "\n"
                              << risk_engine.generateRiskReport(risk_results) << "\n";
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "✗ Risk assessment failed: " << e.what() << std::endl;
            std::cerr << "Continuing with basic analysis..." << std::endl;
        }
    }

    // ========== REPORT RESULTS ==========
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

        // Only show detailed findings if not too many
        if (findings.size() <= 10)
        {
            std::cout << "\nDetailed findings:" << std::endl;
            std::cout << "==========================================" << std::endl;

            for (size_t i = 0; i < findings.size(); i++)
            {
                std::cout << "\nFinding #" << (i + 1) << ":\n";
                std::cout << findings[i].toString() << std::endl;
            }
        }
        else
        {
            std::cout << "\n(Detailed findings omitted due to quantity. Use --output for full report.)" << std::endl;
        }
    }

    // Summary
    std::cout << "\n=== ANALYSIS SUMMARY ===" << std::endl;
    std::cout << "Phases completed: A (Tokenization), B (Parsing), C (Semantic),";
    std::cout << " D (CFG), E (Detection)";
    if (generate_risk_report)
        std::cout << ", I (Risk Assessment)";
    std::cout << std::endl;
    std::cout << "Functions analyzed: " << functions.size() << std::endl;
    std::cout << "CFGs built: " << all_cfgs.size() << std::endl;
    std::cout << "Detectors run: " << detectorManager.getDetectorCount() << std::endl;
    std::cout << "Security issues found: " << findings.size() << std::endl;

    if (generate_risk_report && !findings.empty())
    {
        // Show risk score if available
        std::cout << "Risk Assessment: Completed" << std::endl;
    }

    // Exit code based on findings
    if (!findings.empty())
    {
        // Check for critical findings
        bool has_critical = false;
        for (const auto &finding : findings)
        {
            if (finding.severity == detectors::Severity::CRITICAL)
            {
                has_critical = true;
                break;
            }
        }

        if (has_critical)
        {
            std::cout << "\n✗ CRITICAL vulnerabilities detected. Immediate action required!" << std::endl;
            return 3; // Exit code 3 for critical issues
        }
        else
        {
            std::cout << "\n⚠ Security vulnerabilities detected. Review recommended." << std::endl;
            return 2; // Exit code 2 for non-critical security issues
        }
    }

    std::cout << "\n✓ Analysis completed successfully." << std::endl;
    return 0;
}