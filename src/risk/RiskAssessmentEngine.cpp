// src/risk/RiskAssessmentEngine.cpp
#include "RiskAssessmentEngine.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

namespace risk
{

        // ==================== RiskAssessmentEngine Implementation ====================

    RiskAssessmentEngine::RiskAssessmentEngine(
        std::unique_ptr<RiskScoreCalculator> calculator,
        std::unique_ptr<ComplianceChecker> checker)
        : risk_calculator_(std::move(calculator)), compliance_checker_(std::move(checker))
    {

        // Initialize default components if none provided
        if (!risk_calculator_ || !compliance_checker_)
        {
            initializeDefaultComponents();
        }
    }

    void RiskAssessmentEngine::initializeDefaultComponents()
    {
        if (!risk_calculator_)
        {
            risk_calculator_ = std::make_unique<DefaultRiskScoreCalculator>();
        }

        if (!compliance_checker_)
        {
            compliance_checker_ = std::make_unique<DefaultComplianceChecker>();
        }
    }

    RiskAssessmentResult RiskAssessmentEngine::assessRisk(
        const std::vector<detectors::Finding> &findings,
        const std::vector<std::string> &required_standards,
        detectors::Severity severity_threshold)
    {

        RiskAssessmentResult result;
        result.findings = findings;
        result.total_findings = findings.size();

        // Calculate severity statistics
        calculateSeverityStats(result);

        // Calculate project risk score
        result.project_risk = risk_calculator_->calculateProjectRisk(
            findings, detector_weights_);

        // Calculate per-detector risks
        auto findings_by_detector = groupFindingsByDetector(findings);

        for (const auto &[detector_id, detector_findings] : findings_by_detector)
        {
            double weight = 1.0;
            if (detector_weights_.count(detector_id))
            {
                weight = detector_weights_.at(detector_id);
            }

            result.detector_risks[detector_id] =
                risk_calculator_->calculateDetectorRisk(detector_id, detector_findings, weight);
            result.finding_counts[detector_id] = detector_findings.size();
        }

        // Check compliance
        result.compliance = compliance_checker_->checkCompliance(
            findings, required_standards, severity_threshold);

        // Get compliance statistics
        result.compliance_stats = compliance_checker_->getComplianceStatistics(findings);

        // Generate recommendations
        generateRecommendations(result);

        return result;
    }

    void RiskAssessmentEngine::setDetectorWeights(const std::map<std::string, double> &weights)
    {
        detector_weights_ = weights;
    }

    const std::map<std::string, double> &RiskAssessmentEngine::getDetectorWeights() const
    {
        return detector_weights_;
    }

    void RiskAssessmentEngine::loadWeightsFromPolicy(const std::map<std::string, double> &policy_data)
    {
        detector_weights_ = policy_data;
    }

    std::string RiskAssessmentEngine::generateRiskReport(const RiskAssessmentResult &results) const
    {
        std::ostringstream report;

        report << "=== SECURITY RISK ASSESSMENT REPORT ===\n\n";

        // Executive Summary
        report << "EXECUTIVE SUMMARY\n";
        report << std::string(40, '-') << "\n";
        report << results.getSummary() << "\n";

        // Detailed Risk Scores
        report << "DETAILED RISK ANALYSIS\n";
        report << std::string(40, '-') << "\n";
        report << "Project Risk Score: " << std::fixed << std::setprecision(3)
               << results.project_risk.overall << "/1.0\n";
        report << "  • Likelihood: " << results.project_risk.likelihood << "\n";
        report << "  • Impact: " << results.project_risk.impact << "\n";
        report << "  • Explanation: " << results.project_risk.explanation << "\n\n";

        // Detector-wise Analysis
        if (!results.detector_risks.empty())
        {
            report << "DETECTOR-WISE RISK SCORES\n";
            report << std::string(40, '-') << "\n";

            for (const auto &[detector_id, risk_score] : results.detector_risks)
            {
                int count = results.finding_counts.at(detector_id);
                report << detector_id << " (" << count << " findings):\n";
                report << "  • Score: " << std::fixed << std::setprecision(3)
                       << risk_score.overall << "/1.0\n";
                report << "  • Details: " << risk_score.explanation << "\n\n";
            }
        }

        // Compliance Section
        report << "COMPLIANCE CHECK\n";
        report << std::string(40, '-') << "\n";
        report << results.compliance.report << "\n";

        // Recommendations
        if (!results.recommendations.empty())
        {
            report << "RECOMMENDATIONS\n";
            report << std::string(40, '-') << "\n";
            for (size_t i = 0; i < results.recommendations.size(); ++i)
            {
                report << i + 1 << ". " << results.recommendations[i] << "\n";
            }
        }

        // Technical Details
        report << "\nTECHNICAL DETAILS\n";
        report << std::string(40, '-') << "\n";
        report << "Total findings processed: " << results.total_findings << "\n";
        report << "Detectors triggered: " << results.finding_counts.size() << "\n";
        report << "Risk calculation method: Likelihood × Impact\n";
        report << "Report generated by: C-Code Security Analyzer Risk Assessment Engine\n";

        return report.str();
    }

    std::string RiskAssessmentEngine::generateExecutiveSummary(const RiskAssessmentResult &results) const
    {
        std::ostringstream summary;

        summary << "EXECUTIVE SECURITY RISK SUMMARY\n";
        summary << std::string(40, '=') << "\n\n";

        // Risk Level
        std::string risk_level;
        if (results.project_risk.overall >= 0.8)
        {
            risk_level = "CRITICAL";
        }
        else if (results.project_risk.overall >= 0.6)
        {
            risk_level = "HIGH";
        }
        else if (results.project_risk.overall >= 0.4)
        {
            risk_level = "MEDIUM";
        }
        else if (results.project_risk.overall >= 0.2)
        {
            risk_level = "LOW";
        }
        else
        {
            risk_level = "MINIMAL";
        }

        summary << "OVERALL RISK LEVEL: " << risk_level << "\n";
        summary << "Risk Score: " << std::fixed << std::setprecision(1)
                << (results.project_risk.overall * 100) << "%\n\n";

        // Key Findings
        summary << "KEY FINDINGS:\n";
        summary << "• Total vulnerabilities: " << results.total_findings << "\n";

        if (results.critical_findings > 0)
        {
            summary << "• CRITICAL issues: " << results.critical_findings
                    << " (requires immediate attention)\n";
        }

        if (results.high_findings > 0)
        {
            summary << "• HIGH severity issues: " << results.high_findings << "\n";
        }

        // Compliance Status
        summary << "\nCOMPLIANCE STATUS: ";
        if (results.compliance.passed)
        {
            summary << "✓ PASS\n";
        }
        else
        {
            summary << "✗ FAIL (" << results.compliance.violated_standards.size()
                    << " standards violated)\n";
        }

        // Top Recommendations
        if (!results.recommendations.empty())
        {
            summary << "\nTOP RECOMMENDATIONS:\n";
            for (size_t i = 0; i < std::min(3UL, results.recommendations.size()); ++i)
            {
                summary << i + 1 << ". " << results.recommendations[i] << "\n";
            }
        }

        // Next Steps
        summary << "\nNEXT STEPS:\n";
        if (results.isHighRisk())
        {
            summary << "1. IMMEDIATE ACTION REQUIRED: Address critical/high risk findings\n";
            summary << "2. Review detailed technical report for remediation guidance\n";
            summary << "3. Consider security code review before deployment\n";
        }
        else if (results.total_findings > 0)
        {
            summary << "1. Address medium and high severity findings\n";
            summary << "2. Implement security best practices\n";
            summary << "3. Schedule regular security scans\n";
        }
        else
        {
            summary << "1. Maintain current security practices\n";
            summary << "2. Continue regular security testing\n";
            summary << "3. Consider advanced security controls\n";
        }

        return summary.str();
    }

    void RiskAssessmentEngine::setRiskCalculator(std::unique_ptr<RiskScoreCalculator> calculator)
    {
        risk_calculator_ = std::move(calculator);
    }

    void RiskAssessmentEngine::setComplianceChecker(std::unique_ptr<ComplianceChecker> checker)
    {
        compliance_checker_ = std::move(checker);
    }

    // ==================== Private Helper Methods ====================

    void RiskAssessmentEngine::calculateSeverityStats(RiskAssessmentResult &results) const
    {
        for (const auto &finding : results.findings)
        {
            switch (finding.severity)
            {
            case detectors::Severity::CRITICAL:
                results.critical_findings++;
                break;
            case detectors::Severity::HIGH:
                results.high_findings++;
                break;
            case detectors::Severity::MEDIUM:
                results.medium_findings++;
                break;
            case detectors::Severity::LOW:
                results.low_findings++;
                break;
            case detectors::Severity::INFO:
                results.info_findings++;
                break;
            }
        }
    }

    void RiskAssessmentEngine::generateRecommendations(RiskAssessmentResult &results) const
    {
        results.recommendations.clear();

        // Generate recommendations based on findings

        if (results.critical_findings > 0)
        {
            results.recommendations.push_back(
                "IMMEDIATE ACTION: Fix " + std::to_string(results.critical_findings) +
                " critical vulnerabilities before deployment.");
        }

        if (results.high_findings > 5)
        {
            results.recommendations.push_back(
                "Address " + std::to_string(results.high_findings) +
                " high severity issues in the next sprint.");
        }

        // Check for specific detector patterns
        for (const auto &[detector_id, count] : results.finding_counts)
        {
            if (detector_id == "MEM001" && count > 3)
            {
                results.recommendations.push_back(
                    "Replace unsafe memory functions (strcpy, gets) with safe alternatives.");
            }

            if (detector_id == "TAINT001" && count > 2)
            {
                results.recommendations.push_back(
                    "Implement input validation and sanitization for user-controlled data.");
            }

            if (detector_id == "FMT001" && count > 0)
            {
                results.recommendations.push_back(
                    "Use format string literals or validated format strings.");
            }
        }

        // Compliance recommendations
        if (!results.compliance.passed)
        {
            std::string standards;
            for (const auto &standard : results.compliance.violated_standards)
            {
                if (!standards.empty())
                    standards += ", ";
                standards += standard;
            }
            results.recommendations.push_back(
                "Address compliance violations for: " + standards);
        }

        // General recommendations
        if (results.project_risk.overall > 0.7)
        {
            results.recommendations.push_back(
                "Consider a comprehensive security review due to high overall risk.");
        }

        if (results.total_findings == 0)
        {
            results.recommendations.push_back(
                "No security issues found. Continue regular security testing.");
        }
        else if (results.total_findings < 5)
        {
            results.recommendations.push_back(
                "Minor security issues found. Address them as part of regular maintenance.");
        }
    }

    std::map<std::string, std::vector<detectors::Finding>>
    RiskAssessmentEngine::groupFindingsByDetector(const std::vector<detectors::Finding> &findings) const
    {
        std::map<std::string, std::vector<detectors::Finding>> groups;

        for (const auto &finding : findings)
        {
            groups[finding.rule_id].push_back(finding);
        }

        return groups;
    }

    std::string RiskAssessmentEngine::severityToString(detectors::Severity severity) const
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
    }

} // namespace risk