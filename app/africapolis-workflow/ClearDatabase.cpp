#include <fishnet/MemgraphConnection.hpp>
#include <fishnet/CipherQuery.hpp>
#include <fishnet/TaskConfig.hpp>

#include <CLI/CLI.hpp>

int main(int argc, char const *argv[])
{
    CLI::App app{"AfricapolisClearDatabase"};
    std::string configFilename = "/home/lolo/Documents/fishnet/cwl/africapolis/africapolis-config.json";
    app.add_option("-c,--config", configFilename, "Path to configuration file for graph components stage of Africapolis workflow")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);
    MemgraphTaskConfig config{json::parse(std::ifstream(configFilename))};
    MemgraphConnection connection = MemgraphConnection::create(config.params).value_or_throw();
    connection.executeAndDiscard(CipherQuery::DELETE_ALL());
    return 0;
}
