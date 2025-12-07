// src/risk/RiskAssessmentEngine.hpp
#ifndef RISKASSESSMENTENGINE_HPP
#define RISKASSESSMENTENGINE_HPP

#include "RiskMetrics.hpp" // Get RiskAssessmentResult from here
#include "RiskScoreCalculator.hpp"
#include "ComplianceChecker.hpp"
#include <vector>
#include <memory>
#include <map>
#include <string>

namespace risk
{

    // RiskAssessmentResult is now defined in RiskMetrics.hpp

    class RiskAssessmentEngine
    {
    public:
        RiskAssessmentEngine(
            std::unique_ptr<RiskScoreCalculator> calculator = nullptr,
            std::unique_ptr<ComplianceChecker> checker = nullptr);

        ~RiskAssessmentEngine() = default;

        RiskAssessmentResult assessRisk(
            const std::vector<detectors::Finding> &findings,
            const std::vector<std::string> &required_standards = {},
            detectors::Severity severity_threshold = detectors::Severity::MEDIUM);

        void setDetectorWeights(const std::map<std::string, double> &weights);
        const std::map<std::string, double> &getDetectorWeights() const;
        void loadWeightsFromPolicy(const std::map<std::string, double> &policy_data);

        std::string generateRiskReport(const RiskAssessmentResult &results) const;
        std::string generateExecutiveSummary(const RiskAssessmentResult &results) const;

        void setRiskCalculator(std::unique_ptr<RiskScoreCalculator> calculator);
        void setComplianceChecker(std::unique_ptr<ComplianceChecker> checker);

    private:
        std::unique_ptr<RiskScoreCalculator> risk_calculator_;
        std::unique_ptr<ComplianceChecker> compliance_checker_;
        std::map<std::string, double> detector_weights_;

        void initializeDefaultComponents();
        void calculateSeverityStats(RiskAssessmentResult &results) const;
        void generateRecommendations(RiskAssessmentResult &results) const;
        std::map<std::string, std::vector<detectors::Finding>>
        groupFindingsByDetector(const std::vector<detectors::Finding> &findings) const;
        std::string severityToString(detectors::Severity severity) const;
    };

} // namespace risk

#endif // RISKASSESSMENTENGINE_HPP