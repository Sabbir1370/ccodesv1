// src/risk/ComplianceChecker.cpp
#include "ComplianceChecker.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace risk
{

    // ==================== ComplianceMapping Methods ====================

    // ==================== DefaultComplianceChecker Implementation ====================

    DefaultComplianceChecker::DefaultComplianceChecker(
        const std::map<std::string, ComplianceMapping> &policy_data)
        : rule_mappings_(policy_data)
    {

        if (rule_mappings_.empty())
        {
            initializeDefaultMappings();
        }
    }

    void DefaultComplianceChecker::initializeDefaultMappings()
    {
        // These are fallback mappings if not provided via policy
        rule_mappings_ = {
            {"MEM001", {"CERT-C STR00-C", "OWASP A8:2017", {"CWE-119", "CWE-120", "CWE-787"}, "memory"}},
            {"TAINT001", {"CERT-C MSC00-C", "OWASP A1:2017", {"CWE-89", "CWE-78", "CWE-20"}, "taint"}},
            {"FMT001", {"CERT-C FIO00-C", "OWASP A1:2017", {"CWE-134"}, "format"}},
            {"INIT001", {"CERT-C EXP33-C", "OWASP A9:2017", {"CWE-457"}, "initialization"}},
            {"BUF001", {"CERT-C ARR00-C", "OWASP A2:2017", {"CWE-120", "CWE-787"}, "buffer"}}};
    }

    ComplianceMapping DefaultComplianceChecker::mapFindingToStandards(
        const detectors::Finding &finding) const
    {

        // First check if we have a direct mapping for this rule_id
        auto it = rule_mappings_.find(finding.rule_id);
        if (it != rule_mappings_.end())
        {
            return it->second;
        }

        // If finding already has references, use them
        if (!finding.cert_reference.empty() || !finding.owasp_reference.empty())
        {
            // Extract CWE IDs from rule_id pattern if available
            std::vector<std::string> cwe_ids;
            // Simple heuristic: rule_id ending in numbers might correspond to CWE
            if (finding.rule_id.find("CWE") == 0)
            {
                cwe_ids.push_back(finding.rule_id);
            }

            return ComplianceMapping{
                finding.cert_reference,
                finding.owasp_reference,
                cwe_ids,
                "unknown"};
        }

        // Default: create a generic mapping based on severity
        std::string category;
        switch (finding.severity)
        {
        case detectors::Severity::CRITICAL:
        case detectors::Severity::HIGH:
            category = "high-risk";
            break;
        case detectors::Severity::MEDIUM:
            category = "medium-risk";
            break;
        default:
            category = "low-risk";
        }

        return ComplianceMapping{"", "", {}, category};
    }

    ComplianceResult DefaultComplianceChecker::checkCompliance(
        const std::vector<detectors::Finding> &findings,
        const std::vector<std::string> &required_standards,
        detectors::Severity severity_threshold) const
    {

        ComplianceResult result;

        if (required_standards.empty())
        {
            result.passed = true;
            result.report = "No compliance standards required to check against.";
            return result;
        }

        // Helper function to convert severity to string
        auto severityToString = [](detectors::Severity severity) -> std::string
        {
            switch (severity)
            {
            case detectors::Severity::CRITICAL:
                return "CRITICAL";
            case detectors::Severity::HIGH:
                return "HIGH";
            case detectors::Severity::MEDIUM:
                return "MEDIUM";
            case detectors::Severity::LOW:
                return "LOW";
            case detectors::Severity::INFO:
                return "INFO";
            default:
                return "UNKNOWN";
            }
        };

        std::ostringstream report;
        report << "=== Compliance Check Results ===\n";
        report << "Required standards: ";
        for (size_t i = 0; i < required_standards.size(); ++i)
        {
            if (i > 0)
                report << ", ";
            report << required_standards[i];
        }
        report << "\nSeverity threshold: "
               << severityToString(severity_threshold)
               << " and above\n\n";

        int total_violations = 0;

        // Check each finding against required standards
        for (const auto &finding : findings)
        {
            // Skip findings below severity threshold
            if (finding.severity < severity_threshold)
            {
                continue;
            }

            // Map finding to standards
            auto mapping = mapFindingToStandards(finding);

            // Check against each required standard
            for (const auto &standard : required_standards)
            {
                if (violatesStandard(finding, standard))
                {
                    result.violated_standards.insert(standard);
                    result.standard_violation_counts[standard]++;
                    total_violations++;

                    report << formatViolation(finding, standard) << "\n";
                }
            }
        }

        result.passed = result.violated_standards.empty();

        if (total_violations == 0)
        {
            report << "✓ No compliance violations found.\n";
            result.passed = true;
        }
        else
        {
            report << "\n✗ Found " << total_violations << " compliance violation(s).\n";
            result.passed = false;
        }

        // Add summary
        report << "\n=== Summary ===\n";
        report << "Total findings checked: " << findings.size() << "\n";
        report << "Findings above severity threshold: "
               << std::count_if(findings.begin(), findings.end(),
                                [severity_threshold](const detectors::Finding &f)
                                {
                                    return f.severity >= severity_threshold;
                                })
               << "\n";
        report << "Standards violated: ";

        if (result.violated_standards.empty())
        {
            report << "None";
        }
        else
        {
            bool first = true;
            for (const auto &standard : result.violated_standards)
            {
                if (!first)
                    report << ", ";
                report << standard << " (" << result.standard_violation_counts[standard] << ")";
                first = false;
            }
        }

        result.report = report.str();
        return result;
    }
    // In generateComplianceReport method:
    std::string DefaultComplianceChecker::generateComplianceReport(
        const std::vector<detectors::Finding> &findings) const
    {

        if (findings.empty())
        {
            return "No security findings to analyze for compliance.\n";
        }

        // Helper function to convert severity to string
        auto severityToString = [](detectors::Severity severity) -> std::string
        {
            switch (severity)
            {
            case detectors::Severity::CRITICAL:
                return "CRITICAL";
            case detectors::Severity::HIGH:
                return "HIGH";
            case detectors::Severity::MEDIUM:
                return "MEDIUM";
            case detectors::Severity::LOW:
                return "LOW";
            case detectors::Severity::INFO:
                return "INFO";
            default:
                return "UNKNOWN";
            }
        };

        std::ostringstream report;
        report << "=== COMPLIANCE ANALYSIS REPORT ===\n\n";

        // Group findings by standard
        std::map<std::string, std::vector<const detectors::Finding *>> findings_by_standard;
        std::map<std::string, int> standard_counts;

        for (const auto &finding : findings)
        {
            auto mapping = mapFindingToStandards(finding);

            if (!mapping.cert_reference.empty())
            {
                std::string cert_std = extractStandardName(mapping.cert_reference);
                findings_by_standard[cert_std].push_back(&finding);
                standard_counts[cert_std]++;
            }

            if (!mapping.owasp_reference.empty())
            {
                std::string owasp_std = extractStandardName(mapping.owasp_reference);
                findings_by_standard[owasp_std].push_back(&finding);
                standard_counts[owasp_std]++;
            }

            for (const auto &cwe : mapping.cwe_ids)
            {
                findings_by_standard["CWE"].push_back(&finding);
                standard_counts["CWE"]++;
            }
        }

        // Report by standard
        report << "Findings by Compliance Standard:\n";
        report << std::string(40, '-') << "\n";

        for (const auto &[standard, count] : standard_counts)
        {
            report << std::setw(20) << std::left << standard
                   << ": " << count << " finding(s)\n";
        }

        report << "\n";

        // Detailed breakdown
        report << "Detailed Breakdown:\n";
        report << std::string(40, '-') << "\n";

        for (const auto &[standard, findings_list] : findings_by_standard)
        {
            if (findings_list.empty())
                continue;

            report << "\n"
                   << standard << ":\n";

            // Group by rule_id within this standard
            std::map<std::string, int> rule_counts;
            std::map<std::string, detectors::Severity> rule_max_severity;

            for (const auto *finding : findings_list)
            {
                rule_counts[finding->rule_id]++;
                if (finding->severity > rule_max_severity[finding->rule_id])
                {
                    rule_max_severity[finding->rule_id] = finding->severity;
                }
            }

            for (const auto &[rule_id, count] : rule_counts)
            {
                report << "  • " << std::setw(10) << std::left << rule_id
                       << ": " << count << " finding(s), max severity: "
                       << severityToString(rule_max_severity[rule_id]) << "\n";
            }
        }

        // Recommendations
        report << "\n=== RECOMMENDATIONS ===\n";

        if (findings_by_standard.count("CERT-C") &&
            findings_by_standard["CERT-C"].size() > 5)
        {
            report << "• Many CERT-C violations detected. Consider reviewing:\n";
            report << "  - CERT C Secure Coding Standard\n";
            report << "  - Focus on memory safety and error handling\n";
        }

        if (findings_by_standard.count("OWASP") &&
            findings_by_standard["OWASP"].size() > 3)
        {
            report << "• OWASP Top 10 violations present. Review:\n";
            report << "  - OWASP Application Security Verification Standard\n";
            report << "  - Focus on injection flaws and broken authentication\n";
        }

        if (!findings_by_standard.count("CERT-C") &&
            !findings_by_standard.count("OWASP"))
        {
            report << "• No major compliance standard violations detected.\n";
            report << "  Consider implementing additional security controls.\n";
        }

        return report.str();
    }
    std::map<std::string, int> DefaultComplianceChecker::getComplianceStatistics(
        const std::vector<detectors::Finding> &findings) const
    {

        std::map<std::string, int> stats;

        for (const auto &finding : findings)
        {
            auto mapping = mapFindingToStandards(finding);

            if (!mapping.cert_reference.empty())
            {
                std::string cert_std = extractStandardName(mapping.cert_reference);
                stats[cert_std]++;
            }

            if (!mapping.owasp_reference.empty())
            {
                std::string owasp_std = extractStandardName(mapping.owasp_reference);
                stats[owasp_std]++;
            }

            for (const auto &cwe : mapping.cwe_ids)
            {
                stats["CWE"]++;
            }

            if (!mapping.category.empty())
            {
                stats["Category:" + mapping.category]++;
            }
        }

        // Add total count
        stats["Total Findings"] = findings.size();

        // Count by severity
        std::map<detectors::Severity, int> severity_counts;
        for (const auto &finding : findings)
        {
            severity_counts[finding.severity]++;
        }

        for (const auto &[severity, count] : severity_counts)
        {
            stats["Severity:" + detectors::SeverityToString(severity)] = count;
        }

        return stats;
    }

    void DefaultComplianceChecker::setRuleMapping(
        const std::string &rule_id,
        const ComplianceMapping &mapping)
    {

        rule_mappings_[rule_id] = mapping;
    }

    const std::map<std::string, ComplianceMapping> &
    DefaultComplianceChecker::getMappings() const
    {
        return rule_mappings_;
    }

    // ==================== Private Helper Methods ====================

    std::string DefaultComplianceChecker::extractStandardName(
        const std::string &reference) const
    {

        if (reference.empty())
            return "Unknown";

        // Extract the standard prefix (e.g., "CERT-C" from "CERT-C MEM00-C")
        size_t space_pos = reference.find(' ');
        if (space_pos != std::string::npos)
        {
            return reference.substr(0, space_pos);
        }

        // Check common patterns
        if (reference.find("CERT") == 0)
            return "CERT-C";
        if (reference.find("OWASP") == 0)
            return "OWASP";
        if (reference.find("CWE") == 0)
            return "CWE";
        if (reference.find("ISO") == 0)
            return "ISO";
        if (reference.find("NIST") == 0)
            return "NIST";

        return reference;
    }

    bool DefaultComplianceChecker::violatesStandard(
        const detectors::Finding &finding,
        const std::string &standard) const
    {

        auto mapping = mapFindingToStandards(finding);

        // Check if this finding maps to the required standard
        if (!mapping.cert_reference.empty() &&
            extractStandardName(mapping.cert_reference) == standard)
        {
            return true;
        }

        if (!mapping.owasp_reference.empty() &&
            extractStandardName(mapping.owasp_reference) == standard)
        {
            return true;
        }

        // Check CWE IDs
        for (const auto &cwe : mapping.cwe_ids)
        {
            if (extractStandardName(cwe) == standard)
            {
                return true;
            }
        }

        return false;
    }

    std::string DefaultComplianceChecker::formatViolation(
        const detectors::Finding &finding,
        const std::string &standard) const
    {

        std::ostringstream oss;
        oss << "✗ Violation: " << standard << "\n";
        oss << "  Rule: " << finding.rule_id << "\n";
        oss << "  Location: " << finding.location.toString() << "\n";
        oss << "  Severity: " << finding.severityToString() << "\n";
        oss << "  Description: " << finding.description << "\n";

        // Show mapping if available
        auto mapping = mapFindingToStandards(finding);
        if (!mapping.isEmpty())
        {
            oss << "  Mapped to: " << mapping.toString() << "\n";
        }

        return oss.str();
    }

} // namespace risk
