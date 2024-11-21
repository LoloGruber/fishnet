#pragma once
#include <mgclient.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Common super class for all task configurations.
 * Stores the json used for configuration
 */
struct TaskConfig{
    json jsonDescription;

    TaskConfig(json configDescription):jsonDescription(std::move(configDescription)){}

    TaskConfig()=default;
};

/**
 * @brief Common super class for all task needing a memgraph connection
 * Parses the required memgraph parameters from the json
 */
struct MemgraphTaskConfig : public TaskConfig{
    constexpr static const char * MEMGRAPH_PORT_KEY = "memgraph-port";
    constexpr static const char * MEMGRAPH_HOSTNAME_KEY = "memgraph-host";

    mg::Client::Params params;

    MemgraphTaskConfig()=default;
    MemgraphTaskConfig(const MemgraphTaskConfig&) = default;
    MemgraphTaskConfig& operator=(const MemgraphTaskConfig&) = default;
    virtual ~MemgraphTaskConfig() = default;

    MemgraphTaskConfig(MemgraphTaskConfig&& other) noexcept
        : TaskConfig(std::move(other)), params(std::move(other.params)) {}

    MemgraphTaskConfig& operator=(MemgraphTaskConfig&& other) noexcept {
        if (this != &other) {
            TaskConfig::operator=(std::move(other));
            params = std::move(other.params);
        }
        return *this;
    }

    MemgraphTaskConfig(const json & configDescription):TaskConfig(configDescription){
        std::string hostname;
        u_int16_t port;
        jsonDescription.at(MEMGRAPH_HOSTNAME_KEY).get_to(hostname);
        jsonDescription.at(MEMGRAPH_PORT_KEY).get_to(port);
        setMemgraphParams(std::move(hostname),port);
    }

    void setMemgraphParams(std::string && hostname,uint16_t port) {
        this->params.host = std::move(hostname);
        this->params.port = port;
    }
};


