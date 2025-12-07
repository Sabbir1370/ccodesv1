// src/risk/RiskScoreCalculator.hpp
#ifndef RISKSCORECALCULATOR_HPP
#define RISKSCORECALCULATOR_HPP

#include "RiskMetrics.hpp" // Get RiskScore from here
#include "detectors/Finding.hpp"
#include <vector>
#include <map>
#include <string>

namespace risk
{

    // RiskScore struct is now defined in RiskMetrics.hpp

    class RiskScoreCalculator
    {
    public:
        virtual ~RiskScoreCalculator() = default;

        virtual RiskScore calculateFindingRisk(
            const detectors::Finding &finding,
            double risk_weight = 1.0) const = 0;

        virtual RiskScore calculateProjectRisk(
            const std::vector<detectors::Finding> &findings,
            const std::map<std::string, double> &detector_weights) const = 0;

        virtual RiskScore calculateDetectorRisk(
            const std::string &detector_id,
            const std::vector<detectors::Finding> &findings,
            double risk_weight) const = 0;
    };

    class DefaultRiskScoreCalculator : public RiskScoreCalculator
    {
    public:
        DefaultRiskScoreCalculator() = default;

        RiskScore calculateFindingRisk(
            const detectors::Finding &finding,
            double risk_weight = 1.0) const override;

        RiskScore calculateProjectRisk(
            const std::vector<detectors::Finding> &findings,
            const std::map<std::string, double> &detector_weights) const override;

        RiskScore calculateDetectorRisk(
            const std::string &detector_id,
            const std::vector<detectors::Finding> &findings,
            double risk_weight) const override;

    private:
        double severityToScore(detectors::Severity severity) const;
        double calculateLikelihood(const detectors::Finding &finding) const;
        double calculateImpact(const detectors::Finding &finding, double risk_weight) const;
        std::string generateExplanation(const detectors::Finding &finding,
                                        double likelihood, double impact, double overall) const;
    };

} // namespace risk

#endif // RISKSCORECALCULATOR_HPP