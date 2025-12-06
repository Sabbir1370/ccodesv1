// TaintFlowDetector.cpp - FIXED VERSION
#include "TaintFlowDetector.hpp"
#include <iostream>
#include <sstream>

using namespace detectors;

TaintFlowDetector::TaintFlowDetector()
    : VulnerabilityDetector("TAINT001",
                            "Detects dangerous data flows from untrusted sources to security-sensitive operations")
{
    config_.enabled = true;
    config_.severity_override = Severity::MEDIUM;
    config_.risk_weight = 3;
}
std::vector<Finding> TaintFlowDetector::analyze(
    std::shared_ptr<ASTNode> ast,
    SymbolTable *symtab,
    const std::vector<std::shared_ptr<CFG>> &cfgs)
{
    std::vector<Finding> findings;

    // std::cout << "[TAINT] ===== Starting Taint Analysis =====\n";
    // std::cout << "[TAINT] Number of CFGs to analyze: " << cfgs.size() << "\n";

    for (size_t i = 0; i < cfgs.size(); i++)
    {
        const auto &cfg = cfgs[i];
        if (!cfg)
            continue;

        if (auto func = cfg->getFunction())
        {
            std::string func_name = func->getFunctionName();
            // std::cout << "[TAINT] Function " << i + 1 << ": " << func_name << "\n";

            // Create FunctionContext
            FunctionContext context(cfg);

            // Run analysis
            // std::cout << "[TAINT]   Running taint analysis..." << std::endl;
            auto taint_paths = context.analyze(); // Get actual taint paths

            // Get taint sources (without duplicates)
            auto sources = context.getTaintSources();
            // std::cout << "[TAINT]   Found " << sources.size() << " unique taint source(s)\n";

            // Create findings for taint sources
            for (const auto &source : sources)
            {
                Finding finding(
                    "TAINT001",
                    "Tainted data source: " + source.identifier + " (from user input)",
                    source.source_location,
                    Severity::MEDIUM);
                finding.function_name = func_name;
                finding.cert_reference = "CERT-C MSC24-C";
                finding.owasp_reference = "OWASP A1:2017";
                findings.push_back(finding);
            }

            // Create findings for taint PATHS (source -> sink)
            // This is where we report actual flows
            if (!taint_paths.empty())
            {
                // std::cout << "[TAINT]   Found " << taint_paths.size()
                //          << " taint flow path(s)\n";

                for (const auto &path : taint_paths)
                {
                    const auto &source = path.first;
                    const auto &sink = path.second;

                    Finding flow_finding(
                        "TAINT002",
                        "Taint flow detected: " + source.identifier +
                            " → " + sink.identifier,
                        sink.source_location, // Report at sink location
                        Severity::HIGH);
                    flow_finding.function_name = func_name;
                    flow_finding.variable_name = source.identifier;
                    flow_finding.cert_reference = "CERT-C MSC24-C";
                    flow_finding.owasp_reference = "OWASP A1:2017";

                    // Add trace (source -> sink)
                    flow_finding.addTraceLocation(source.source_location);
                    flow_finding.addTraceLocation(sink.source_location);

                    findings.push_back(flow_finding);
                }
            }

            // Debug output
            std::string taint_graph = context.getTaintGraph();
            if (!taint_graph.empty())
            {
                // std::cout << "[TAINT]   Taint Graph:\n"
                //          << taint_graph << std::endl;
            }
        }
    }

    // std::cout << "[TAINT] ===== Taint Analysis Complete =====\n";
    // std::cout << "[TAINT] Total findings: " << findings.size() << "\n";

    return findings;
}