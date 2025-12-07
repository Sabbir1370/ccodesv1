// src/risk/RiskMetrics.hpp
#ifndef RISKMETRICS_HPP
#define RISKMETRICS_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <sstream> // Add this for std::ostringstream
#include <iomanip> // Add this for std::fixed, std::setprecision

// Forward declarations
namespace detectors
{
    struct Finding;
    enum class Severity;
}

namespace risk
{

    // ==================== CORE DATA STRUCTURES ====================

    struct RiskScore
    {
        double overall;          // Final computed risk score (0.0-1.0)
        double likelihood;       // Probability of exploitation (0.0-1.0)
        double impact;           // Severity of impact (0.0-1.0)
        std::string explanation; // How score was calculated

        RiskScore() : overall(0.0), likelihood(0.0), impact(0.0) {}
        RiskScore(double o, double l, double i, const std::string &e)
            : overall(o), likelihood(l), impact(i), explanation(e) {}
    };

    struct ComplianceMapping
    {
        std::string cert_reference;       // e.g., "CERT-C MEM00-C"
        std::string owasp_reference;      // e.g., "OWASP A1:2017"
        std::vector<std::string> cwe_ids; // e.g., ["CWE-119", "CWE-120"]
        std::string category;             // e.g., "memory", "taint", "format"

        ComplianceMapping() = default;
        ComplianceMapping(const std::string &cert,
                          const std::string &owasp,
                          const std::vector<std::string> &cwes,
                          const std::string &cat)
            : cert_reference(cert), owasp_reference(owasp), cwe_ids(cwes), category(cat) {}

        /**
         * @brief Check if mapping is empty (no compliance standards mapped)
         */
        bool isEmpty() const
        {
            return cert_reference.empty() && owasp_reference.empty() && cwe_ids.empty();
        }

        /**
         * @brief Get string representation of the mapping
         */
        std::string toString() const
        {
            std::ostringstream oss;
            bool has_content = false;

            if (!cert_reference.empty())
            {
                oss << "CERT: " << cert_reference;
                has_content = true;
            }

            if (!owasp_reference.empty())
            {
                if (has_content)
                    oss << ", ";
                oss << "OWASP: " << owasp_reference;
                has_content = true;
            }

            if (!cwe_ids.empty())
            {
                if (has_content)
                    oss << ", ";
                oss << "CWE: ";
                for (size_t i = 0; i < cwe_ids.size(); ++i)
                {
                    if (i > 0)
                        oss << ", ";
                    oss << cwe_ids[i];
                }
            }

            if (!category.empty())
            {
                if (has_content)
                    oss << " ";
                oss << "[" << category << "]";
            }

            return oss.str();
        }
    };

    struct ComplianceResult
    {
        bool passed;                                          // True if no violations of required standards
        std::set<std::string> violated_standards;             // Which standards were violated
        std::map<std::string, int> standard_violation_counts; // Count per standard
        std::string report;                                   // Detailed compliance report

        ComplianceResult() : passed(true) {}
    };

    struct RiskAssessmentResult
    {
        // Original findings
        std::vector<detectors::Finding> findings;

        // Risk scores
        RiskScore project_risk;
        std::map<std::string, RiskScore> detector_risks; // key: detector_id

        // Compliance results
        ComplianceResult compliance;

        // Statistics
        std::map<std::string, int> finding_counts;   // key: detector_id -> count
        std::map<std::string, int> severity_counts;  // key: severity -> count
        std::map<std::string, int> compliance_stats; // key: standard -> count

        // Summary information
        int total_findings;
        int critical_findings;
        int high_findings;
        int medium_findings;
        int low_findings;
        int info_findings;

        // Recommendations
        std::vector<std::string> recommendations;

        RiskAssessmentResult()
            : total_findings(0), critical_findings(0), high_findings(0),
              medium_findings(0), low_findings(0), info_findings(0) {}

        /**
         * @brief Get formatted summary string
         */
        std::string getSummary() const
        {
            std::ostringstream oss;
            oss << "Risk Assessment Summary:\n";
            oss << "=======================\n";
            oss << "Total Findings: " << total_findings << "\n";
            oss << "Critical: " << critical_findings << ", High: " << high_findings
                << ", Medium: " << medium_findings << ", Low: " << low_findings
                << ", Info: " << info_findings << "\n";
            oss << "Project Risk Score: " << std::fixed << std::setprecision(3)
                << project_risk.overall << " (Likelihood: " << project_risk.likelihood
                << ", Impact: " << project_risk.impact << ")\n";
            oss << "Compliance: " << (compliance.passed ? "PASS" : "FAIL") << "\n";
            oss << "Detectors with findings: " << finding_counts.size() << "\n";

            if (!recommendations.empty())
            {
                oss << "Top Recommendation: " << recommendations.front() << "\n";
            }

            return oss.str();
        }

        /**
         * @brief Check if assessment indicates high risk
         */
        bool isHighRisk() const
        {
            // High risk if:
            // 1. Project risk score > 0.7, OR
            // 2. Has critical findings, OR
            // 3. More than 5 high severity findings
            return project_risk.overall > 0.7 ||
                   critical_findings > 0 ||
                   high_findings > 5;
        }

        /**
         * @brief Check if compliance requirements are met
         */
        bool isCompliant() const
        {
            return compliance.passed;
        }
    };

} // namespace risk

#endif // RISKMETRICS_HPP