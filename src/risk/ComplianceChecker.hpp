// src/risk/ComplianceChecker.hpp
#ifndef COMPLIANCECHECKER_HPP
#define COMPLIANCECHECKER_HPP

#include "RiskMetrics.hpp" // Get ComplianceMapping and ComplianceResult from here
#include "detectors/Finding.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>

namespace risk
{

    // ComplianceMapping and ComplianceResult are now defined in RiskMetrics.hpp

    class ComplianceChecker
    {
    public:
        virtual ~ComplianceChecker() = default;

        virtual ComplianceMapping mapFindingToStandards(
            const detectors::Finding &finding) const = 0;

        virtual ComplianceResult checkCompliance(
            const std::vector<detectors::Finding> &findings,
            const std::vector<std::string> &required_standards,
            detectors::Severity severity_threshold = detectors::Severity::MEDIUM) const = 0;

        virtual std::string generateComplianceReport(
            const std::vector<detectors::Finding> &findings) const = 0;

        virtual std::map<std::string, int> getComplianceStatistics(
            const std::vector<detectors::Finding> &findings) const = 0;
    };

    class DefaultComplianceChecker : public ComplianceChecker
    {
    public:
        explicit DefaultComplianceChecker(
            const std::map<std::string, ComplianceMapping> &policy_data = {});

        ComplianceMapping mapFindingToStandards(
            const detectors::Finding &finding) const override;

        ComplianceResult checkCompliance(
            const std::vector<detectors::Finding> &findings,
            const std::vector<std::string> &required_standards,
            detectors::Severity severity_threshold = detectors::Severity::MEDIUM) const override;

        std::string generateComplianceReport(
            const std::vector<detectors::Finding> &findings) const override;

        std::map<std::string, int> getComplianceStatistics(
            const std::vector<detectors::Finding> &findings) const override;

        void setRuleMapping(const std::string &rule_id, const ComplianceMapping &mapping);
        const std::map<std::string, ComplianceMapping> &getMappings() const;

    private:
        std::map<std::string, ComplianceMapping> rule_mappings_;
        void initializeDefaultMappings();
        std::string extractStandardName(const std::string &reference) const;
        bool violatesStandard(const detectors::Finding &finding, const std::string &standard) const;
        std::string formatViolation(const detectors::Finding &finding, const std::string &standard) const;
    };

} // namespace risk

#endif // COMPLIANCECHECKER_HPP