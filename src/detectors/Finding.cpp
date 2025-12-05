// src/detectors/Finding.cpp
#include "Finding.hpp"
#include <sstream>
#include <iomanip>

namespace detectors
{

    std::string Finding::severityToString() const
    {
        switch (severity)
        {
        case Severity::CRITICAL:
            return "CRITICAL";
        case Severity::HIGH:
            return "HIGH";
        case Severity::MEDIUM:
            return "MEDIUM";
        case Severity::LOW:
            return "LOW";
        case Severity::INFO:
            return "INFO";
        default:
            return "UNKNOWN";
        }
    }

    std::string Finding::toString() const
    {
        std::ostringstream oss;

        oss << "[ " << std::left << std::setw(8) << severityToString() << " ] "
            << rule_id << ": " << description << "\n";

        oss << "    Location: " << location.toString();
        if (!function_name.empty())
        {
            oss << " in function '" << function_name << "'";
        }
        oss << "\n";

        if (!variable_name.empty())
        {
            oss << "    Variable: " << variable_name << "\n";
        }

        if (!cert_reference.empty())
        {
            oss << "    CERT: " << cert_reference << "\n";
        }

        if (!owasp_reference.empty())
        {
            oss << "    OWASP: " << owasp_reference << "\n";
        }

        if (hasTrace())
        {
            oss << "    Trace (" << trace.size() << " locations):\n";
            for (size_t i = 0; i < trace.size(); ++i)
            {
                oss << "      " << (i + 1) << ". " << trace[i].toString() << "\n";
            }
        }

        return oss.str();
    }

    void Finding::addTraceLocation(const SourceLocation &loc)
    {
        trace.push_back(loc);
    }

} // namespace detectors