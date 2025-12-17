#pragma once
#include <mgclient.hpp>
#include <optional>
#include <fishnet/PathHelper.h>
#include <fishnet/TaskConfig.hpp>

namespace testutil {
class WorkflowTestEnvironment {
private:
    std::filesystem::path configFilePath;
public:
    static mg::Client::Params memgraphParams() {
        auto getEnvOrEmpty = [](const char * envVar) {
            const char * value = std::getenv(envVar);
            return value ? std::optional<std::string>(value) : std::nullopt;
        };
        auto mapToUint16 = [](const std::string & str) -> std::optional<uint16_t> {
            try {
                size_t pos;
                int value = std::stoi(str, &pos);
                if (pos == str.size() && value >= 0 && value <= 65535) {
                    return static_cast<uint16_t>(value);
                }
            } catch (const std::exception &) {
                // Ignore conversion errors
            }
            return std::nullopt;
        };
        return mg::Client::Params{
            .host= getEnvOrEmpty("MEMGRAPH_HOST").value_or("localhost"),
            .port= getEnvOrEmpty("MEMGRAPH_PORT").and_then(mapToUint16).value_or(7687),
            .username= getEnvOrEmpty("MEMGRAPH_USER").value_or(""),
            .password= getEnvOrEmpty("MEMGRAPH_PASSWORD").value_or(""),
            .use_ssl= false,
        };
    }
};
}