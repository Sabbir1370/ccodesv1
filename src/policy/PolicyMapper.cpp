// src/policy/PolicyMapper.cpp
#include "PolicyMapper.hpp"
#include "../utils/json.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace policy
{

    bool PolicyMapper::loadFromFile(const std::string &file_path)
    {
        try
        {
            std::ifstream file(file_path);
            if (!file.is_open())
            {
                std::cerr << "PolicyMapper: Cannot open policy file: " << file_path << std::endl;
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            bool success = loadFromString(buffer.str());
            if (success)
            {
                policy_file_path_ = file_path;
                std::cout << "PolicyMapper: Successfully loaded policy from " << file_path << std::endl;
            }

            return success;
        }
        catch (const std::exception &e)
        {
            std::cerr << "PolicyMapper: Error loading file " << file_path
                      << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool PolicyMapper::loadFromString(const std::string &json_content)
    {
        try
        {
            bool success = parseJSON(json_content);
            is_loaded_ = success;
            return success;
        }
        catch (const std::exception &e)
        {
            std::cerr << "PolicyMapper: Error parsing JSON: " << e.what() << std::endl;
            is_loaded_ = false;
            return false;
        }
    }

    bool PolicyMapper::parseJSON(const std::string &json_content)
    {
        try
        {
            auto j = nlohmann::json::parse(json_content);

            // Clear existing policies
            policy_map_.clear();

            // Check if we have a detectors section
            if (j.contains("detectors") && j["detectors"].is_object())
            {
                const auto &detectors = j["detectors"];

                for (const auto &[detector_name, detector_config] : detectors.items())
                {
                    DetectorPolicy policy;

                    // Get enabled status
                    if (detector_config.contains("enabled"))
                    {
                        policy.enabled = detector_config["enabled"].get<bool>();
                    }

                    // Get severity
                    if (detector_config.contains("severity"))
                    {
                        std::string severity_str = detector_config["severity"].get<std::string>();
                        policy.severity_override = stringToSeverity(severity_str);
                    }

                    // Get risk weight
                    if (detector_config.contains("risk_weight"))
                    {
                        if (detector_config["risk_weight"].is_number())
                        {
                            policy.risk_weight = detector_config["risk_weight"].get<int>();
                        }
                        else if (detector_config["risk_weight"].is_string())
                        {
                            std::string weight_str = detector_config["risk_weight"].get<std::string>();
                            try
                            {
                                policy.risk_weight = std::stoi(weight_str);
                            }
                            catch (...)
                            {
                                policy.risk_weight = 1;
                            }
                        }
                    }

                    // Get description
                    if (detector_config.contains("description"))
                    {
                        policy.description = detector_config["description"].get<std::string>();
                    }

                    // Store the policy
                    policy_map_[detector_name] = policy;
                }
            }
            else
            {
                // No detectors section, use defaults
                setDefaultPolicies();
            }

            return true;
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "PolicyMapper: JSON parse error: " << e.what() << std::endl;
            setDefaultPolicies(); // Fall back to defaults
            return false;
        }
        catch (const std::exception &e)
        {
            std::cerr << "PolicyMapper: Error parsing policy: " << e.what() << std::endl;
            setDefaultPolicies(); // Fall back to defaults
            return false;
        }
    }

    detectors::Severity PolicyMapper::stringToSeverity(const std::string &severity_str) const
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

    void PolicyMapper::setDefaultPolicies()
    {
        // Set default policies for your known detectors
        policy_map_["SecureMemTracker"] = DetectorPolicy(
            true, detectors::Severity::HIGH, 1,
            "Detects unsafe memory/string function usage");

        policy_map_["TaintFlowDetector"] = DetectorPolicy(
            true, detectors::Severity::CRITICAL, 2,
            "Tracks tainted data through program flow");

        policy_map_["FormatStringInspector"] = DetectorPolicy(
            true, detectors::Severity::MEDIUM, 1,
            "Checks for format string vulnerabilities");

        policy_map_["UseBeforeInitDetector"] = DetectorPolicy(
            true, detectors::Severity::MEDIUM, 1,
            "Detects uninitialized variable usage");

        policy_map_["SimpleBufferDetector"] = DetectorPolicy(
            false, detectors::Severity::LOW, 1,
            "Basic buffer overflow heuristics");
    }

    bool PolicyMapper::isDetectorEnabled(const std::string &detector_name) const
    {
        auto it = policy_map_.find(detector_name);
        if (it != policy_map_.end())
        {
            return it->second.enabled;
        }

        // If not in policy, check if it's one of our known detectors
        if (detector_name == "SecureMemTracker" ||
            detector_name == "TaintFlowDetector" ||
            detector_name == "FormatStringInspector" ||
            detector_name == "UseBeforeInitDetector")
        {
            return true; // Default enabled
        }

        return false; // Unknown detector, disabled by default
    }

    detectors::Severity PolicyMapper::getSeverityOverride(const std::string &detector_name) const
    {
        auto it = policy_map_.find(detector_name);
        if (it != policy_map_.end())
        {
            return it->second.severity_override;
        }

        // Default severities for known detectors
        if (detector_name == "SecureMemTracker")
            return detectors::Severity::HIGH;
        else if (detector_name == "TaintFlowDetector")
            return detectors::Severity::CRITICAL;
        else if (detector_name == "FormatStringInspector")
            return detectors::Severity::MEDIUM;
        else if (detector_name == "UseBeforeInitDetector")
            return detectors::Severity::MEDIUM;
        else if (detector_name == "SimpleBufferDetector")
            return detectors::Severity::LOW;
        else
            return detectors::Severity::MEDIUM;
    }

    int PolicyMapper::getRiskWeight(const std::string &detector_name) const
    {
        auto it = policy_map_.find(detector_name);
        if (it != policy_map_.end())
        {
            return it->second.risk_weight;
        }

        // Default weights
        if (detector_name == "SecureMemTracker")
            return 1;
        else if (detector_name == "TaintFlowDetector")
            return 2;
        else
            return 1;
    }

    std::string PolicyMapper::getDetectorDescription(const std::string &detector_name) const
    {
        auto it = policy_map_.find(detector_name);
        if (it != policy_map_.end() && !it->second.description.empty())
        {
            return it->second.description;
        }

        // Default descriptions
        if (detector_name == "SecureMemTracker")
            return "Unsafe memory/string function usage";
        else if (detector_name == "TaintFlowDetector")
            return "Tainted data flow analysis";
        else if (detector_name == "FormatStringInspector")
            return "Format string vulnerability detection";
        else if (detector_name == "UseBeforeInitDetector")
            return "Uninitialized variable usage detection";
        else if (detector_name == "SimpleBufferDetector")
            return "Basic buffer overflow heuristics";
        else
            return "Security vulnerability detector";
    }

    void PolicyMapper::applyToDetector(detectors::VulnerabilityDetector &detector) const
    {
        std::string name = detector.getName();

        // Create config from policy
        detectors::DetectorConfig config;
        config.enabled = isDetectorEnabled(name);
        config.severity_override = getSeverityOverride(name);
        config.risk_weight = getRiskWeight(name);

        // Apply to detector
        detector.setConfig(config);
    }

    std::vector<std::string> PolicyMapper::getAllDetectorNames() const
    {
        std::vector<std::string> names;
        names.reserve(policy_map_.size());

        for (const auto &[name, _] : policy_map_)
        {
            names.push_back(name);
        }

        return names;
    }

    std::vector<std::string> PolicyMapper::getEnabledDetectorNames() const
    {
        std::vector<std::string> names;

        for (const auto &[name, policy] : policy_map_)
        {
            if (policy.enabled)
            {
                names.push_back(name);
            }
        }

        return names;
    }

} // namespace policy