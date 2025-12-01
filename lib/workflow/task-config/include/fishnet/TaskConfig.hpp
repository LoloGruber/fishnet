#pragma once
#include <mgclient.hpp>
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

/**
 * @brief Common super class for all task needing a memgraph connection
 * Parses the required memgraph parameters from the json
 */
struct MemgraphTaskConfig : public TaskConfig{
    constexpr static const char * MEMGRAPH_PORT_KEY = "memgraph-port";
    constexpr static const char * MEMGRAPH_HOSTNAME_KEY = "memgraph-host";
    constexpr static const char * MEMGRAPH_USE_SSL_KEY = "memgraph-use-ssl";
    constexpr static const char * MEMGRAPH_USERNAME_KEY = "memgraph-user";
    constexpr static const char * MEMGRAPH_PASSWORD_KEY = "memgraph-password";

    mg::Client::Params params;

    MemgraphTaskConfig(const json & configDescription):TaskConfig(configDescription){
        jsonDescription.at(MEMGRAPH_HOSTNAME_KEY).get_to(params.host);
        jsonDescription.at(MEMGRAPH_PORT_KEY).get_to(params.port);
        set_or_else(jsonDescription,MEMGRAPH_USE_SSL_KEY,params.use_ssl,false);
        set_or_else(jsonDescription,MEMGRAPH_USERNAME_KEY,params.username,"");
        set_or_else(jsonDescription,MEMGRAPH_PASSWORD_KEY,params.password,"");
    }
private:
    template<typename T>
    inline void set_or_else(const json & jsonDescription, const char * key, T & target, std::convertible_to<T> auto defaultValue){
        if(jsonDescription.contains(key)){
            jsonDescription.at(key).get_to(target);
        } else {
            target = defaultValue;
        }
    }

};
