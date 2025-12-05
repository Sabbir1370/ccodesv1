#include "DetectorManager.hpp"
#include "detectors/SecureMemTracker.hpp"
#include "detectors/TaintFlowDetector.hpp"
#include "detectors/FormatStringInspector.hpp"
#include <algorithm>
#include <iostream>

namespace detectors
{

    void DetectorManager::registerDetector(std::unique_ptr<VulnerabilityDetector> detector)
    {
        if (!detector)
            return;
        detectors_.push_back(std::move(detector));
    }

    void DetectorManager::enableDetector(const std::string &rule_id)
    {
        for (auto &detector : detectors_)
        {
            if (detector->getName() == rule_id)
            {
                detector->setEnabled(true);
                return;
            }
        }
    }

    void DetectorManager::disableDetector(const std::string &rule_id)
    {
        for (auto &detector : detectors_)
        {
            if (detector->getName() == rule_id)
            {
                detector->setEnabled(false);
                return;
            }
        }
    }

    std::vector<Finding> DetectorManager::runAllDetectors(
        std::shared_ptr<ASTNode> ast,
        SymbolTable *symtab,
        const std::vector<std::shared_ptr<CFG>> &cfgs // CFG, not cfg::CFG
    )
    {
        std::vector<Finding> allFindings;

        for (auto &detector : detectors_)
        {
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
        const std::vector<std::shared_ptr<CFG>> &cfgs // CFG, not cfg::CFG
    )
    {
        std::vector<Finding> allFindings;

        for (auto &detector : detectors_)
        {
            if (detector->isEnabled())
            {
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