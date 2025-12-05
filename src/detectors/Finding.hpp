// src/detectors/Finding.hpp
#ifndef FINDING_HPP
#define FINDING_HPP

#include <string>
#include <vector>
#include "utils/SourceLocation.hpp"

namespace detectors
{

    /**
     * @brief Severity levels for vulnerability findings
     */
    enum class Severity
    {
        CRITICAL, // Immediate threat, exploitable
        HIGH,     // Serious vulnerability
        MEDIUM,   // Moderate risk
        LOW,      // Minor issue or best practice violation
        INFO      // Informational only
    };

    /**
     * @brief Represents a single security vulnerability finding
     */
    struct Finding
    {
        std::string rule_id;     // e.g., "MEM001", "TAINT001"
        std::string description; // Human-readable description
        SourceLocation location; // Source location of the issue
        Severity severity;       // Severity level

        // Additional metadata for compliance mapping
        std::string cert_reference;  // e.g., "CERT-C MEM00-C"
        std::string owasp_reference; // e.g., "OWASP A1:2017"

        // For taint analysis and traceback
        std::vector<SourceLocation> trace; // Path from source to sink

        // Context information
        std::string function_name; // Function where issue was found
        std::string variable_name; // Related variable (if any)

        /**
         * @brief Construct a new Finding object
         */
        Finding() = default;

        /**
         * @brief Construct a new Finding object with basic info
         */
        Finding(const std::string &rule_id,
                const std::string &description,
                const SourceLocation &location,
                Severity severity)
            : rule_id(rule_id),
              description(description),
              location(location),
              severity(severity),
              function_name(""),
              variable_name("") {}

        /**
         * @brief Get severity as string
         */
        std::string severityToString() const;

        /**
         * @brief Get finding as formatted string for reporting
         */
        std::string toString() const;

        /**
         * @brief Add a location to the trace path
         */
        void addTraceLocation(const SourceLocation &loc);

        /**
         * @brief Check if finding has a trace
         */
        bool hasTrace() const { return !trace.empty(); }
    };

} // namespace detectors

#endif // FINDING_HPP