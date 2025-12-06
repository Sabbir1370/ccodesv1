// src/policy/RuleRepository.hpp - FIXED
#ifndef RULE_REPOSITORY_HPP
#define RULE_REPOSITORY_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "detectors/VulnerabilityDetector.hpp"

namespace policy
{

    /**
     * @brief Represents a complete rule for a detector
     */
    struct DetectorRule
    {
        std::string id;                                             // Detector ID (e.g., "MEM001")
        std::string name;                                           // Detector name (e.g., "SecureMemTracker")
        bool enabled = true;                                        // Whether detector is enabled
        detectors::Severity severity = detectors::Severity::MEDIUM; // Base severity
        int risk_weight = 1;                                        // Risk multiplier
        std::string description;                                    // Human-readable description
        std::string cert_reference;                                 // CERT-C reference
        std::string owasp_reference;                                // OWASP reference

        // Metadata
        std::string category;             // e.g., "memory", "taint", "format"
        std::vector<std::string> cwe_ids; // CWE identifiers

        DetectorRule() = default;

        DetectorRule(const std::string &id,
                     const std::string &name,
                     const std::string &desc = "")
            : id(id), name(name), description(desc) {}

        /**
         * @brief Convert severity to numeric score
         */
        int getSeverityScore() const;

        /**
         * @brief Get severity as string
         */
        std::string getSeverityString() const;

        /**
         * @brief Check if rule has compliance references
         */
        bool hasComplianceInfo() const
        {
            return !cert_reference.empty() || !owasp_reference.empty();
        }
    };

    /**
     * @brief Repository for storing and retrieving detector rules
     */
    class RuleRepository
    {
    private:
        std::unordered_map<std::string, DetectorRule> rules_by_name_;
        std::unordered_map<std::string, DetectorRule> rules_by_id_;
        bool initialized_ = false;

    public:
        RuleRepository() = default;

        /**
         * @brief Initialize repository from JSON string
         */
        bool initializeFromJSON(const std::string &json_content);

        /**
         * @brief Load built-in default rules
         */
        void loadDefaultRules();

        /**
         * @brief Add or update a rule
         */
        void addRule(const DetectorRule &rule);

        /**
         * @brief Get rule by detector name
         */
        DetectorRule getRuleByName(const std::string &detector_name) const;

        /**
         * @brief Get rule by detector ID
         */
        DetectorRule getRuleById(const std::string &detector_id) const;

        /**
         * @brief Check if detector is enabled by name
         */
        bool isDetectorEnabled(const std::string &detector_name) const;

        /**
         * @brief Check if detector is enabled by ID
         */
        bool isDetectorEnabledById(const std::string &detector_id) const;

        /**
         * @brief Get all detector rules
         */
        std::vector<DetectorRule> getAllRules() const;

        /**
         * @brief Get only enabled detector rules
         */
        std::vector<DetectorRule> getEnabledRules() const;

        /**
         * @brief Get rules by category
         */
        std::vector<DetectorRule> getRulesByCategory(const std::string &category) const;

        /**
         * @brief Get all detector names
         */
        std::vector<std::string> getAllDetectorNames() const;

        /**
         * @brief Get enabled detector names
         */
        std::vector<std::string> getEnabledDetectorNames() const;

        /**
         * @brief Check if repository is initialized
         */
        bool isInitialized() const { return initialized_; }

        /**
         * @brief Get number of rules in repository
         */
        size_t size() const { return rules_by_name_.size(); }

        /**
         * @brief Clear all rules
         */
        void clear();

    private:
        /**
         * @brief Convert string to Severity enum
         */
        detectors::Severity stringToSeverity(const std::string &severity_str) const;

        /**
         * @brief Initialize internal mappings
         */
        void rebuildMappings();

        // REMOVED: parseJSON and parseRuleFromJSON methods - they should only be in .cpp
    };

} // namespace policy

#endif // RULE_REPOSITORY_HPP