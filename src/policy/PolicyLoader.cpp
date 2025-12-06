// src/policy/PolicyLoader.cpp - CORRECT VERSION (NO getJSON())
#include "PolicyLoader.hpp"
#include "../utils/json.hpp" // Include ONLY in .cpp
#include <fstream>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

namespace policy
{

    // Implementation class
    class PolicyLoader::Impl
    {
    public:
        json jsonData;
        bool loaded = false;
        std::string filePath;

        bool parseFromString(const std::string &json_content)
        {
            try
            {
                jsonData = json::parse(json_content);
                loaded = true;
                return true;
            }
            catch (const json::parse_error &e)
            {
                std::cerr << "PolicyLoader: JSON parse error: " << e.what() << std::endl;
                loaded = false;
                return false;
            }
        }
    };

    // Public methods
    PolicyLoader::PolicyLoader() : pImpl_(std::make_unique<Impl>()) {}

    PolicyLoader::~PolicyLoader() = default;

    PolicyLoader::PolicyLoader(PolicyLoader &&other) noexcept = default;
    PolicyLoader &PolicyLoader::operator=(PolicyLoader &&other) noexcept = default;

    bool PolicyLoader::loadFromFile(const std::string &file_path)
    {
        try
        {
            std::ifstream file(file_path);
            if (!file.is_open())
            {
                std::cerr << "PolicyLoader: Cannot open file: " << file_path << std::endl;
                return false;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            bool success = pImpl_->parseFromString(buffer.str());
            if (success)
            {
                pImpl_->filePath = file_path;
                std::cout << "PolicyLoader: Successfully loaded policy from "
                          << file_path << std::endl;
            }

            return success;
        }
        catch (const std::exception &e)
        {
            std::cerr << "PolicyLoader: Error loading file " << file_path
                      << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool PolicyLoader::loadFromString(const std::string &json_content)
    {
        return pImpl_->parseFromString(json_content);
    }

    std::string PolicyLoader::getRawJSON() const
    {
        if (!pImpl_->loaded)
        {
            return "{}";
        }
        return pImpl_->jsonData.dump(2);
    }

    std::string PolicyLoader::getJSONString() const
    {
        return getRawJSON();
    }

    bool PolicyLoader::isLoaded() const
    {
        return pImpl_->loaded;
    }

    std::string PolicyLoader::getFilePath() const
    {
        return pImpl_->filePath;
    }

    // REMOVED: getJSON() method - we don't need to expose nlohmann::json

} // namespace policy