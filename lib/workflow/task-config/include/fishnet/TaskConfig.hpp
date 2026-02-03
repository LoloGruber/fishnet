#pragma once
#include <nlohmann/json.hpp>
#include <concepts>

using json = nlohmann::json;

/**
 * @brief Common super class for all task configurations.
 * Stores the json used for configuration
 */
struct TaskConfig{
    constexpr static const char * WORKING_DIRECTORY_KEY = "working-directory";
    json jsonDescription;
    std::filesystem::path workingDirectory;

    TaskConfig(json configDescription):jsonDescription(std::move(configDescription)){
        if(this->jsonDescription.contains(WORKING_DIRECTORY_KEY)){
            this->jsonDescription.at(WORKING_DIRECTORY_KEY).get_to(this->workingDirectory);
        }

    }

    TaskConfig()=default;
};
