#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

class ConfigJsonReader{
protected:
    json config;
public:
    explicit ConfigJsonReader(const std::string & jsonString){
        config = json::parse(jsonString);
    }

    explicit ConfigJsonReader(const std::filesystem::path & pathToConfig) {
        config = json::parse(std::ifstream(pathToConfig));
    }
};