// src/policy/RuleRepository.cpp - FIXED
#include "RuleRepository.hpp"
#include "PolicyLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Include json.hpp ONLY in .cpp file
#include "../utils/json.hpp"

// Use the namespace
using json = nlohmann::json;

namespace policy
{

    // Helper functions for JSON parsing (only in .cpp)
    namespace
    {
        bool parseRuleFromJSONImpl(RuleRepository &repo, const json &rule_json,
                                   const std::string &detector_name)
        {
            try
            {
                DetectorRule rule;
                rule.name = detector_name;

                // Get rule ID
                if (rule_json.contains("rule_id"))
                {
                    rule.id = rule_json["rule_id"].get<std::string>();
                }
                else
                {
                    // Generate ID from name if not provided
                    rule.id = detector_name;
                    std::replace(rule.id.begin(), rule.id.end(), ' ', '_');
                }

                // Get enabled status
                if (rule_json.contains("enabled"))
                {
                    rule.enabled = rule_json["enabled"].get<bool>();
                }

                // Get severity - FIXED VERSION
                if (rule_json.contains("severity"))
                {
                    std::string severity_str = rule_json["severity"].get<std::string>();
                    // Convert directly
                    std::string lower_severity = severity_str;
                    std::transform(lower_severity.begin(), lower_severity.end(),
                                   lower_severity.begin(), ::tolower);

                    if (lower_severity == "critical" || lower_severity == "crit")
                        rule.severity = detectors::Severity::CRITICAL;
                    else if (lower_severity == "high")
                        rule.severity = detectors::Severity::HIGH;
                    else if (lower_severity == "medium" || lower_severity == "med")
                        rule.severity = detectors::Severity::MEDIUM;
                    else if (lower_severity == "low")
                        rule.severity = detectors::Severity::LOW;
                    else if (lower_severity == "info" || lower_severity == "informational")
                        rule.severity = detectors::Severity::INFO;
                    else
                        rule.severity = detectors::Severity::MEDIUM; // Default
                }

                // Get risk weight
                if (rule_json.contains("risk_weight"))
                {
                    if (rule_json["risk_weight"].is_number())
                    {
                        rule.risk_weight = rule_json["risk_weight"].get<int>();
                    }
                }

                // Get description
                if (rule_json.contains("description"))
                {
                    rule.description = rule_json["description"].get<std::string>();
                }

                // Get CERT reference
                if (rule_json.contains("cert_reference"))
                {
                    rule.cert_reference = rule_json["cert_reference"].get<std::string>();
                }

                // Get OWASP reference
                if (rule_json.contains("owasp_reference"))
                {
                    rule.owasp_reference = rule_json["owasp_reference"].get<std::string>();
                }

                // Get category
                if (rule_json.contains("category"))
                {
                    rule.category = rule_json["category"].get<std::string>();
                }

                // Get CWE IDs
                if (rule_json.contains("cwe_ids") && rule_json["cwe_ids"].is_array())
                {
                    for (const auto &cwe_id : rule_json["cwe_ids"])
                    {
                        rule.cwe_ids.push_back(cwe_id.get<std::string>());
                    }
                }

                // Add the rule
                repo.addRule(rule);
                return true;
            }
            catch (const std::exception &e)
            {
                std::cerr << "RuleRepository: Error parsing rule for '"
                          << detector_name << "': " << e.what() << std::endl;
                return false;
            }
        }

        bool parseJSONImpl(RuleRepository &repo, const json &j)
        {
            try
            {
                repo.clear();

                // Check if we have a detectors section
                if (!j.contains("detectors") || !j["detectors"].is_object())
                {
                    std::cerr << "RuleRepository: No 'detectors' section found" << std::endl;
                    repo.loadDefaultRules(); // Fall back to defaults
                    return false;
                }

                const auto &detectors = j["detectors"];
                bool any_rules_parsed = false;

                for (const auto &[detector_name, rule_json] : detectors.items())
                {
                    if (parseRuleFromJSONImpl(repo, rule_json, detector_name))
                    {
                        any_rules_parsed = true;
                    }
                }

                if (!any_rules_parsed)
                {
                    std::cerr << "RuleRepository: No valid rules parsed from JSON" << std::endl;
                    repo.loadDefaultRules(); // Fall back to defaults
                    return false;
                }

                std::cout << "RuleRepository: Successfully parsed "
                          << repo.size() << " rules from JSON" << std::endl;

                return true;
            }
            catch (const std::exception &e)
            {
                std::cerr << "RuleRepository: Error parsing JSON: " << e.what() << std::endl;
                repo.loadDefaultRules(); // Fall back to defaults
                return false;
            }
        }

    }

    // Public method implementations
    bool RuleRepository::initializeFromJSON(const std::string &json_content)
    {
        try
        {
            auto j = json::parse(json_content);
            return parseJSONImpl(*this, j);
        }
        catch (const json::parse_error &e)
        {
            std::cerr << "RuleRepository: JSON parse error: " << e.what() << std::endl;
            loadDefaultRules(); // Fall back to defaults
            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "RuleRepository: Error parsing JSON: " << e.what() << std::endl;
            loadDefaultRules(); // Fall back to defaults
            return false;
        }
    }
    void RuleRepository::loadDefaultRules()
    {
        clear();

        // SecureMemTracker (matches your MEM001)
        DetectorRule mem001("MEM001", "SecureMemTracker",
                            "Unsafe memory/string function usage");
        mem001.severity = detectors::Severity::HIGH;
        mem001.risk_weight = 1;
        mem001.cert_reference = "CERT-C STR00-C";
        mem001.owasp_reference = "OWASP A8:2017";
        mem001.category = "memory";
        mem001.cwe_ids = {"CWE-119", "CWE-120", "CWE-787"};
        addRule(mem001);

        // TaintFlowDetector
        DetectorRule taint001("TAINT001", "TaintFlowDetector",
                              "Tainted data flow analysis");
        taint001.severity = detectors::Severity::CRITICAL;
        taint001.risk_weight = 2;
        taint001.cert_reference = "CERT-C MSC00-C";
        taint001.owasp_reference = "OWASP A1:2017";
        taint001.category = "taint";
        taint001.cwe_ids = {"CWE-89", "CWE-78", "CWE-20"};
        addRule(taint001);

        // FormatStringInspector
        DetectorRule fmt001("FMT001", "FormatStringInspector",
                            "Format string vulnerability detection");
        fmt001.severity = detectors::Severity::MEDIUM;
        fmt001.risk_weight = 1;
        fmt001.cert_reference = "CERT-C FIO00-C";
        fmt001.owasp_reference = "OWASP A1:2017";
        fmt001.category = "format";
        fmt001.cwe_ids = {"CWE-134"};
        addRule(fmt001);

        // UseBeforeInitDetector
        DetectorRule init001("INIT001", "UseBeforeInitDetector",
                             "Uninitialized variable usage detection");
        init001.severity = detectors::Severity::MEDIUM;
        init001.risk_weight = 1;
        init001.cert_reference = "CERT-C EXP33-C";
        init001.owasp_reference = "OWASP A9:2017";
        init001.category = "initialization";
        init001.cwe_ids = {"CWE-457"};
        addRule(init001);

        // SimpleBufferDetector
        DetectorRule buf001("BUF001", "SimpleBufferDetector",
                            "Basic buffer overflow heuristics");
        buf001.severity = detectors::Severity::LOW;
        buf001.risk_weight = 1;
        buf001.enabled = false; // Disabled by default
        buf001.cert_reference = "CERT-C ARR00-C";
        buf001.owasp_reference = "OWASP A2:2017";
        buf001.category = "buffer";
        buf001.cwe_ids = {"CWE-120", "CWE-787"};
        addRule(buf001);

        initialized_ = true;
        std::cout << "RuleRepository: Loaded " << rules_by_name_.size()
                  << " default rules" << std::endl;
    }

    void RuleRepository::addRule(const DetectorRule &rule)
    {
        // Add to both mappings
        rules_by_name_[rule.name] = rule;
        rules_by_id_[rule.id] = rule;

        // Ensure internal consistency
        rules_by_name_[rule.name].id = rule.id;
        rules_by_id_[rule.id].name = rule.name;

        initialized_ = true;
    }

    DetectorRule RuleRepository::getRuleByName(const std::string &detector_name) const
    {
        auto it = rules_by_name_.find(detector_name);
        if (it != rules_by_name_.end())
        {
            return it->second;
        }

        // Return empty rule if not found
        return DetectorRule();
    }

    DetectorRule RuleRepository::getRuleById(const std::string &detector_id) const
    {
        auto it = rules_by_id_.find(detector_id);
        if (it != rules_by_id_.end())
        {
            return it->second;
        }

        // Return empty rule if not found
        return DetectorRule();
    }

    bool RuleRepository::isDetectorEnabled(const std::string &detector_name) const
    {
        auto it = rules_by_name_.find(detector_name);
        if (it != rules_by_name_.end())
        {
            return it->second.enabled;
        }

        // Default to enabled for known detectors
        return true;
    }

    bool RuleRepository::isDetectorEnabledById(const std::string &detector_id) const
    {
        auto it = rules_by_id_.find(detector_id);
        if (it != rules_by_id_.end())
        {
            return it->second.enabled;
        }

        // Default to enabled for known detectors
        return true;
    }

    std::vector<DetectorRule> RuleRepository::getAllRules() const
    {
        std::vector<DetectorRule> rules;
        rules.reserve(rules_by_name_.size());

        for (const auto &[name, rule] : rules_by_name_)
        {
            rules.push_back(rule);
        }

        return rules;
    }

    std::vector<DetectorRule> RuleRepository::getEnabledRules() const
    {
        std::vector<DetectorRule> rules;

        for (const auto &[name, rule] : rules_by_name_)
        {
            if (rule.enabled)
            {
                rules.push_back(rule);
            }
        }

        return rules;
    }

    std::vector<DetectorRule> RuleRepository::getRulesByCategory(const std::string &category) const
    {
        std::vector<DetectorRule> rules;

        for (const auto &[name, rule] : rules_by_name_)
        {
            if (rule.category == category)
            {
                rules.push_back(rule);
            }
        }

        return rules;
    }

    std::vector<std::string> RuleRepository::getAllDetectorNames() const
    {
        std::vector<std::string> names;
        names.reserve(rules_by_name_.size());

        for (const auto &[name, _] : rules_by_name_)
        {
            names.push_back(name);
        }

        return names;
    }

    std::vector<std::string> RuleRepository::getEnabledDetectorNames() const
    {
        std::vector<std::string> names;

        for (const auto &[name, rule] : rules_by_name_)
        {
            if (rule.enabled)
            {
                names.push_back(name);
            }
        }

        return names;
    }

    void RuleRepository::clear()
    {
        rules_by_name_.clear();
        rules_by_id_.clear();
        initialized_ = false;
    }

    detectors::Severity RuleRepository::stringToSeverity(const std::string &severity_str) const
    {
        std::string lower_str = severity_str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);

        if (lower_str == "critical" || lower_str == "crit")
            return detectors::Severity::CRITICAL;
        else if (lower_str == "high")
            return detectors::Severity::HIGH;
        else if (lower_str == "medium" || lower_str == "med")
            return detectors::Severity::MEDIUM;
        else if (lower_str == "low")
            return detectors::Severity::LOW;
        else if (lower_str == "info" || lower_str == "informational")
            return detectors::Severity::INFO;
        else
            return detectors::Severity::MEDIUM; // Default
    }

    void RuleRepository::rebuildMappings()
    {
        rules_by_id_.clear();

        for (const auto &[name, rule] : rules_by_name_)
        {
            rules_by_id_[rule.id] = rule;
        }
    }

} // namespace policy