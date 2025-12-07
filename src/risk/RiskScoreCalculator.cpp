// src/risk/RiskScoreCalculator.cpp
#include "RiskScoreCalculator.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace risk
{

    // ==================== DefaultRiskScoreCalculator Implementation ====================

    RiskScore DefaultRiskScoreCalculator::calculateFindingRisk(
        const detectors::Finding &finding,
        double risk_weight) const
    {

        // Calculate individual components
        double likelihood = calculateLikelihood(finding);
        double impact = calculateImpact(finding, risk_weight);

        // Overall risk = likelihood × impact (classic risk formula)
        double overall = likelihood * impact;

        // Clamp to valid range [0.0, 1.0]
        overall = std::max(0.0, std::min(1.0, overall));

        return RiskScore{
            overall,
            likelihood,
            impact,
            generateExplanation(finding, likelihood, impact, overall)};
    }

    RiskScore DefaultRiskScoreCalculator::calculateProjectRisk(
        const std::vector<detectors::Finding> &findings,
        const std::map<std::string, double> &detector_weights) const
    {

        if (findings.empty())
        {
            return RiskScore{0.0, 0.0, 0.0, "No security findings detected"};
        }

        // Group findings by detector
        std::map<std::string, std::vector<detectors::Finding>> findings_by_detector;
        for (const auto &finding : findings)
        {
            findings_by_detector[finding.rule_id].push_back(finding);
        }

        // Calculate weighted average of detector risks
        double total_weighted_risk = 0.0;
        double total_weights = 0.0;
        double total_likelihood = 0.0;
        double total_impact = 0.0;
        int total_findings = findings.size();

        for (const auto &[detector_id, detector_findings] : findings_by_detector)
        {
            double weight = 1.0;
            if (detector_weights.count(detector_id))
            {
                weight = detector_weights.at(detector_id);
            }

            auto detector_risk = calculateDetectorRisk(detector_id, detector_findings, weight);

            // Weight by number of findings from this detector
            double detector_weight = weight * detector_findings.size();
            total_weighted_risk += detector_risk.overall * detector_weight;
            total_weights += detector_weight;

            total_likelihood += detector_risk.likelihood * detector_findings.size();
            total_impact += detector_risk.impact * detector_findings.size();
        }

        // Calculate averages
        double overall = total_weights > 0 ? total_weighted_risk / total_weights : 0.0;
        double avg_likelihood = total_findings > 0 ? total_likelihood / total_findings : 0.0;
        double avg_impact = total_findings > 0 ? total_impact / total_findings : 0.0;

        std::ostringstream explanation;
        explanation << "Project risk calculated from " << findings.size()
                    << " findings across " << findings_by_detector.size()
                    << " detectors. Weighted average risk: "
                    << std::fixed << std::setprecision(3) << overall;

        return RiskScore{overall, avg_likelihood, avg_impact, explanation.str()};
    }

    RiskScore DefaultRiskScoreCalculator::calculateDetectorRisk(
        const std::string &detector_id,
        const std::vector<detectors::Finding> &findings,
        double risk_weight) const
    {

        if (findings.empty())
        {
            return RiskScore{0.0, 0.0, 0.0, "No findings for detector " + detector_id};
        }

        // Calculate average risk for this detector's findings
        double total_risk = 0.0;
        double total_likelihood = 0.0;
        double total_impact = 0.0;

        for (const auto &finding : findings)
        {
            auto risk = calculateFindingRisk(finding, risk_weight);
            total_risk += risk.overall;
            total_likelihood += risk.likelihood;
            total_impact += risk.impact;
        }

        double count = static_cast<double>(findings.size());
        double avg_risk = total_risk / count;
        double avg_likelihood = total_likelihood / count;
        double avg_impact = total_impact / count;

        std::ostringstream explanation;
        explanation << "Detector " << detector_id << ": " << findings.size()
                    << " findings, average risk: " << std::fixed << std::setprecision(3) << avg_risk;

        return RiskScore{avg_risk, avg_likelihood, avg_impact, explanation.str()};
    }

    // ==================== Private Helper Methods ====================

    double DefaultRiskScoreCalculator::severityToScore(detectors::Severity severity) const
    {
        switch (severity)
        {
        case detectors::Severity::CRITICAL:
            return 1.0;
        case detectors::Severity::HIGH:
            return 0.8;
        case detectors::Severity::MEDIUM:
            return 0.5;
        case detectors::Severity::LOW:
            return 0.3;
        case detectors::Severity::INFO:
            return 0.1;
        default:
            return 0.0;
        }
    }

    double DefaultRiskScoreCalculator::calculateLikelihood(const detectors::Finding &finding) const
    {
        double likelihood = 0.5; // Base likelihood

        // Factor 1: Trace depth - deeper data flow paths are more exploitable
        if (finding.hasTrace())
        {
            // More steps in trace = higher likelihood
            double trace_factor = std::min(0.3, static_cast<double>(finding.trace.size()) * 0.05);
            likelihood += trace_factor;
        }

        // Factor 2: Function context
        if (!finding.function_name.empty())
        {
            // Entry points and handlers are more likely targets
            const std::string &func = finding.function_name;
            if (func == "main" ||
                func.find("handler") != std::string::npos ||
                func.find("callback") != std::string::npos ||
                func.find("process") != std::string::npos)
            {
                likelihood += 0.2;
            }
        }

        // Factor 3: Variable scope (global variables are more accessible)
        if (!finding.variable_name.empty())
        {
            // Simple heuristic: globals often start with 'g_' or are in global scope
            if (finding.variable_name.find("g_") == 0 ||
                finding.variable_name.find("global") == 0)
            {
                likelihood += 0.1;
            }
        }

        // Clamp to valid range
        return std::max(0.0, std::min(1.0, likelihood));
    }

    double DefaultRiskScoreCalculator::calculateImpact(const detectors::Finding &finding, double risk_weight) const
    {
        // Start with severity-based impact
        double impact = severityToScore(finding.severity);

        // Apply risk weight from policy (typically 0.5-2.0)
        impact *= risk_weight;

        // Adjust based on CERT/OWASP references (known high-impact categories)
        if (!finding.cert_reference.empty())
        {
            if (finding.cert_reference.find("MEM") != std::string::npos ||
                finding.cert_reference.find("STR") != std::string::npos)
            {
                // Memory/string vulnerabilities have high impact
                impact *= 1.2;
            }
        }

        // Clamp to valid range
        return std::max(0.0, std::min(1.0, impact));
    }

    std::string DefaultRiskScoreCalculator::generateExplanation(
        const detectors::Finding &finding,
        double likelihood,
        double impact,
        double overall) const
    {

        std::ostringstream explanation;
        explanation << "Rule " << finding.rule_id;

        if (!finding.function_name.empty())
        {
            explanation << " in function '" << finding.function_name << "'";
        }

        explanation << ": likelihood=" << std::fixed << std::setprecision(2) << likelihood
                    << " (based on trace depth and context), "
                    << "impact=" << std::fixed << std::setprecision(2) << impact
                    << " (severity " << finding.severityToString() << "), "
                    << "overall risk=" << std::fixed << std::setprecision(2) << overall
                    << " (likelihood × impact)";

        return explanation.str();
    }

} // namespace risk