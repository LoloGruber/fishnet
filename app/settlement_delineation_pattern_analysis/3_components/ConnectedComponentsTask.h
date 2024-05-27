#include <fishnet/GraphFactory.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include "MemgraphClient.hpp"
#include "Task.hpp"
#include "ConnectedComponentsConfig.hpp"

class ConnectedComponentsTask: public Task{
private:
    ConnectedComponentsConfig config;
public:
    ConnectedComponentsTask(ConnectedComponentsConfig && config):config(std::move(config)){

    }

    void run() override {
        auto ExpMemgraphConnection = MemgraphClient::create(config.params);
        const auto & memgraphConnection = getExpectedOrThrowError(ExpMemgraphConnection);
        auto nodesList = memgraphConnection.nodes();
        auto adjMap = memgraphConnection.edges();
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<size_t>();
        graph.addNodes(nodesList);
        for(auto && [node,neigbours]:adjMap){ 
            for(auto neighbour: neigbours){ // use by value since datatype is very cheap to copy
                graph.addEdge(node,neighbour); //
            }
        }
        auto components = fishnet::graph::BFS::connectedComponents(graph).get();
        auto componentIds = memgraphConnection.createComponents(components);
        


    }
};