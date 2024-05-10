#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <concepts>

using json = nlohmann::json;

class BaseJsonConfigReader{
protected:
    json config;
public:
    explicit BaseJsonConfigReader(const std::string & jsonString){
        config = json::parse(jsonString);
    }

    explicit BaseJsonConfigReader(const std::filesystem::path & pathToConfig) {
        config = json::parse(std::ifstream(pathToConfig));
    }
};

template<typename Reader,typename TaskType>
concept JsonConfigReader = std::derived_from<Reader,BaseJsonConfigReader> &&  std::derived_from<TaskType,Task> && requires(Reader reader, TaskType & taskRef){
    {reader.parse(taskRef)};
};