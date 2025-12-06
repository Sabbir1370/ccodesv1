// src/detectors/DetectorManager.cpp - UPDATED
#include "DetectorManager.hpp"
#include "../policy/PolicyMapper.hpp"
#include "../policy/RuleRepository.hpp"
#include "detectors/SecureMemTracker.hpp"
#include "detectors/TaintFlowDetector.hpp"
#include "detectors/FormatStringInspector.hpp"
#include "detectors/UseBeforeInitDetector.hpp"
#include "detectors/SimpleBufferDetector.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>

namespace detectors
{

    DetectorManager::DetectorManager()
        : policy_mapper_(std::make_shared<policy::PolicyMapper>()),
          rule_repo_(std::make_shared<policy::RuleRepository>())
    {
        initialize();
    }

    void DetectorManager::initialize()
    {
        // Register all available detectors
        registerDetector(std::make_unique<SecureMemTracker>());
        registerDetector(std::make_unique<TaintFlowDetector>());
        registerDetector(std::make_unique<FormatStringInspector>());
        registerDetector(std::make_unique<UseBeforeInitDetector>());
        registerDetector(std::make_unique<SimpleBufferDetector>());

        std::cout << "DetectorManager: Initialized with "
                  << detectors_.size() << " detectors" << std::endl;
    }

    bool DetectorManager::loadPolicy(const std::string &policy_file_path)
    {
        std::cout << "DetectorManager: Attempting to load policy from '"
                  << policy_file_path << "'" << std::endl;

        // Try to load the policy file
        if (!policy_mapper_->loadFromFile(policy_file_path))
        {
            // Try alternative paths
            std::vector<std::string> possible_paths = {
                policy_file_path,
                "../" + policy_file_path,
                "../config/policy.json",
                "../../config/policy.json",
                "config/policy.json"};

            bool loaded = false;
            for (const auto &path : possible_paths)
            {
                if (policy_mapper_->loadFromFile(path))
                {
                    std::cout << "DetectorManager: Found policy at '" << path << "'" << std::endl;
                    loaded = true;
                    break;
                }
            }

            if (!loaded)
            {
                std::cerr << "DetectorManager: Could not load policy file. Using defaults." << std::endl;
                return false;
            }
        }

        // Apply policy to all detectors
        for (auto &detector : detectors_)
        {
            policy_mapper_->applyToDetector(*detector);

            std::string name = detector->getName();
            bool enabled = detector->isEnabled();
            std::cout << "  - " << name << ": "
                      << (enabled ? "ENABLED" : "DISABLED") << std::endl;
        }

        // Initialize rule repository from policy
        if (policy_mapper_->isLoaded())
        {
            // Note: You may need to get the JSON content from PolicyMapper
            // For now, we'll just note that rule repository needs initialization
            std::cout << "DetectorManager: Policy loaded successfully" << std::endl;
        }

        return true;
    }

    std::shared_ptr<policy::PolicyMapper> DetectorManager::getPolicyMapper() const
    {
        return policy_mapper_;
    }

    std::shared_ptr<policy::RuleRepository> DetectorManager::getRuleRepository() const
    {
        return rule_repo_;
    }

    void DetectorManager::registerDetector(std::unique_ptr<VulnerabilityDetector> detector)
    {
        if (!detector)
        {
            std::cerr << "DetectorManager: Cannot register null detector" << std::endl;
            return;
        }

        std::string name = detector->getName();

        // Check if detector already registered
        for (const auto &existing : detectors_)
        {
            if (existing->getName() == name)
            {
                std::cerr << "DetectorManager: Detector '" << name
                          << "' already registered" << std::endl;
                return;
            }
        }

        detectors_.push_back(std::move(detector));
        std::cout << "DetectorManager: Registered detector '" << name << "'" << std::endl;
    }

    void DetectorManager::enableDetector(const std::string &rule_id)
    {
        for (auto &detector : detectors_)
        {
            if (detector->getName() == rule_id)
            {
                detector->setEnabled(true);
                std::cout << "DetectorManager: Enabled detector '" << rule_id << "'" << std::endl;
                return;
            }
        }
        std::cerr << "DetectorManager: Detector '" << rule_id << "' not found" << std::endl;
    }

    void DetectorManager::disableDetector(const std::string &rule_id)
    {
        for (auto &detector : detectors_)
        {
            if (detector->getName() == rule_id)
            {
                detector->setEnabled(false);
                std::cout << "DetectorManager: Disabled detector '" << rule_id << "'" << std::endl;
                return;
            }
        }
        std::cerr << "DetectorManager: Detector '" << rule_id << "' not found" << std::endl;
    }

    std::vector<Finding> DetectorManager::runAllDetectors(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        std::vector<Finding> allFindings;

        for (auto &detector : detectors_)
        {
            std::cout << "DetectorManager: Running detector '"
                      << detector->getName() << "'" << std::endl;

            auto findings = detector->analyze(ast, symtab, cfgs);
            allFindings.insert(allFindings.end(),
                               std::make_move_iterator(findings.begin()),
                               std::make_move_iterator(findings.end()));
        }

        return allFindings;
    }

    std::vector<Finding> DetectorManager::runEnabledDetectors(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs)
    {
        std::vector<Finding> allFindings;

        for (auto &detector : detectors_)
        {
            if (detector->isEnabled())
            {
                std::cout << "DetectorManager: Running enabled detector '"
                          << detector->getName() << "'" << std::endl;

                auto findings = detector->analyze(ast, symtab, cfgs);
                allFindings.insert(allFindings.end(),
                                   std::make_move_iterator(findings.begin()),
                                   std::make_move_iterator(findings.end()));
            }
        }

        return allFindings;
    }

    VulnerabilityDetector *DetectorManager::getDetector(const std::string &rule_id)
    {
        for (auto &detector : detectors_)
        {
            if (detector->getName() == rule_id)
            {
                return detector.get();
            }
        }
        return nullptr;
    }

} // namespace detectors