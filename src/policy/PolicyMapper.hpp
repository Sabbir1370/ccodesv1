// src/policy/PolicyMapper.hpp
#ifndef POLICY_MAPPER_HPP
#define POLICY_MAPPER_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include "detectors/VulnerabilityDetector.hpp"

namespace policy
{

    /**
     * @brief Maps detector names to their configuration policies
     */
    class PolicyMapper
    {
    private:
        struct DetectorPolicy
        {
            bool enabled = true;
            detectors::Severity severity_override = detectors::Severity::MEDIUM;
            int risk_weight = 1;
            std::string description;

            DetectorPolicy() = default;

            DetectorPolicy(bool enabled,
                           detectors::Severity severity,
                           int risk_weight,
                           const std::string &desc = "")
                : enabled(enabled),
                  severity_override(severity),
                  risk_weight(risk_weight),
                  description(desc) {}
        };

        std::unordered_map<std::string, DetectorPolicy> policy_map_;
        std::string policy_file_path_;
        bool is_loaded_ = false;

    public:
        PolicyMapper() = default;

        /**
         * @brief Load policy configuration from a JSON file
         */
        bool loadFromFile(const std::string &file_path);

        /**
         * @brief Load policy configuration from JSON string
         */
        bool loadFromString(const std::string &json_content);

        /**
         * @brief Check if a detector is enabled
         */
        bool isDetectorEnabled(const std::string &detector_name) const;

        /**
         * @brief Get severity override for a detector
         */
        detectors::Severity getSeverityOverride(const std::string &detector_name) const;

        /**
         * @brief Get risk weight for a detector
         */
        int getRiskWeight(const std::string &detector_name) const;

        /**
         * @brief Get description for a detector from policy
         */
        std::string getDetectorDescription(const std::string &detector_name) const;

        /**
         * @brief Apply policy to a detector (modifies detector's config)
         */
        void applyToDetector(detectors::VulnerabilityDetector &detector) const;

        /**
         * @brief Get all detector names in the policy
         */
        std::vector<std::string> getAllDetectorNames() const;

        /**
         * @brief Get only enabled detector names
         */
        std::vector<std::string> getEnabledDetectorNames() const;

        /**
         * @brief Check if policy was successfully loaded
         */
        bool isLoaded() const { return is_loaded_; }

        /**
         * @brief Get the path to the loaded policy file
         */
        std::string getPolicyFilePath() const { return policy_file_path_; }

    private:
        /**
         * @brief Parse JSON content and populate policy_map_
         */
        bool parseJSON(const std::string &json_content);

        /**
         * @brief Convert severity string to Severity enum
         */
        detectors::Severity stringToSeverity(const std::string &severity_str) const;

        /**
         * @brief Set default policies for known detectors
         */
        void setDefaultPolicies();
    };

} // namespace policy

#endif // POLICY_MAPPER_HPP