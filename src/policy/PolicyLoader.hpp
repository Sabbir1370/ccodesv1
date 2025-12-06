// src/policy/PolicyLoader.hpp - CLEAN VERSION
#ifndef POLICY_LOADER_HPP
#define POLICY_LOADER_HPP

#include <string>
#include <memory>

namespace policy
{

    /**
     * @brief Loads and manages policy JSON files
     */
    class PolicyLoader
    {
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl_;

    public:
        PolicyLoader();
        ~PolicyLoader();

        // Non-copyable
        PolicyLoader(const PolicyLoader &) = delete;
        PolicyLoader &operator=(const PolicyLoader &) = delete;

        // Move operations
        PolicyLoader(PolicyLoader &&) noexcept;
        PolicyLoader &operator=(PolicyLoader &&) noexcept;

        /**
         * @brief Load policy from file
         */
        bool loadFromFile(const std::string &file_path);

        /**
         * @brief Load policy from JSON string
         */
        bool loadFromString(const std::string &json_content);

        /**
         * @brief Get raw JSON content as string
         */
        std::string getRawJSON() const;

        /**
         * @brief Check if policy is loaded
         */
        bool isLoaded() const;

        /**
         * @brief Get file path of loaded policy
         */
        std::string getFilePath() const;

        /**
         * @brief Get JSON as string for RuleRepository to parse
         */
        std::string getJSONString() const;
    };

} // namespace policy

#endif // POLICY_LOADER_HPP