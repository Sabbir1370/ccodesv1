// src/detectors/DetectorManager.hpp - UPDATED
#ifndef DETECTOR_MANAGER_HPP
#define DETECTOR_MANAGER_HPP

#include <memory>
#include <vector>
#include <string>
#include "VulnerabilityDetector.hpp"
#include "detectors/TaintFlowDetector.hpp"
#include "detectors/SecureMemTracker.hpp"
#include "detectors/FormatStringInspector.hpp"
#include "detectors/UseBeforeInitDetector.hpp"
#include "detectors/SimpleBufferDetector.hpp"
#include "Finding.hpp"

// Forward declarations to avoid circular includes
namespace policy
{
    class PolicyMapper;
    class RuleRepository;
}

namespace detectors
{

    class DetectorManager
    {
    private:
        std::vector<std::unique_ptr<VulnerabilityDetector>> detectors_;
        std::shared_ptr<policy::PolicyMapper> policy_mapper_;
        std::shared_ptr<policy::RuleRepository> rule_repo_;

    public:
        DetectorManager();
        ~DetectorManager() = default;

        // Non-copyable
        DetectorManager(const DetectorManager &) = delete;
        DetectorManager &operator=(const DetectorManager &) = delete;

        // Move operations
        DetectorManager(DetectorManager &&) = default;
        DetectorManager &operator=(DetectorManager &&) = default;

        // Initialize all detectors
        void initialize();

        // Load and apply policy
        bool loadPolicy(const std::string &policy_file_path);

        // Get policy components
        std::shared_ptr<policy::PolicyMapper> getPolicyMapper() const;
        std::shared_ptr<policy::RuleRepository> getRuleRepository() const;

        void registerDetector(std::unique_ptr<VulnerabilityDetector> detector);

        void enableDetector(const std::string &rule_id);
        void disableDetector(const std::string &rule_id);

        std::vector<Finding> runAllDetectors(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs // CFG, not cfg::CFG
        );

        std::vector<Finding> runEnabledDetectors(
            std::shared_ptr<ASTNode> ast,
            SymbolTable *symtab,
            const std::vector<std::shared_ptr<CFG>> &cfgs // CFG, not cfg::CFG
        );

        size_t getDetectorCount() const { return detectors_.size(); }

        VulnerabilityDetector *getDetector(const std::string &rule_id);

        const std::vector<std::unique_ptr<VulnerabilityDetector>> &getDetectors() const
        {
            return detectors_;
        }
    };

} // namespace detectors

#endif // DETECTOR_MANAGER_HPP