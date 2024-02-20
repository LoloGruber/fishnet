#include <gtest/gtest.h>
#include "Contraction.h"
#include "Graph.h"
#include "XYNode.h"
#include "GraphTestUtil.h"
#include "StopWatch.h"
#include <queue>
#include <fstream>
#include "WeightedGraph.h"
#define PERFORMANCE_TEST true

using namespace fishnet::graph;

struct MergePredicate{
    bool operator()(const XYNode & n1, const XYNode & n2)const{
        return n1.distanceTo(n2) <= 1.0;
    }
};

 struct MergeFunction{
    XYNode operator()(const XYNode & n1, const XYNode & n2)const {
        return XYNode((n1.getX()+n2.getX())/2,(n1.getY() + n2.getY())/2);
    }
};

struct DistanceFunction{
    double operator()(const XYNode & n1, const XYNode & n2) const {
        return n1.distanceTo(n2);
    }
};

struct EdgeLessThan{
    bool operator()(const WeightedEdge auto  & e1, const WeightedEdge auto & e2) const {
        return e1.getWeight() < e2.getWeight();
    }
};

struct EdgeGreaterThan{
    bool operator()(const WeightedEdge auto & e1, const WeightedEdge auto & e2) const {
        return e1.getWeight() > e2.getWeight();
    }
};


using WG = Weighted<UndirectedGraph<XYNode>,double,DistanceFunction>;
static void legacyMerge( WG& copy, std::function<bool(const XYNode &,const XYNode &)> Predicate){
    MergeFunction Merger;

    using E = typename WG::edge_type;
    //auto f =  [](const E & e1, const E & e2){
    //    return e1.getWeight() < e2.getWeight();
    //};
    std::priority_queue<E,std::vector<E>, EdgeLessThan> edgesToRemove;
    //Find edges connecting two settlements with distance smaller than the merge distance
    for (auto &edge: copy.getEdges()) {
        if (Predicate(edge.getFrom(),edge.getTo())) {
            edgesToRemove.push(edge);
        }
    }
    
    /* While there are still edges to contract perform while loop */
    while (not edgesToRemove.empty()) { 
        auto edge = edgesToRemove.top();
        edgesToRemove.pop();
        auto u = edge.getFrom();
        auto v = edge.getTo();
        auto s = Merger(u,v); //merge settlements according to mergeStrategy
        #if PERFORMANCE_TEST
        std::this_thread::sleep_for(std::chrono::microseconds(5390));
        #endif
        copy.addNode(s);
        copy.removeEdge(edge);
        auto uFormerEdges = copy.getOutboundEdges(u);
        auto vFormerEdges = copy.getOutboundEdges(v);
        /* Store all former of edges of u and v in a list*/
        auto formerEdges = std::vector<E>();
        formerEdges.reserve(uFormerEdges.size() + vFormerEdges.size());
        for (auto &e: uFormerEdges) {
            formerEdges.push_back(e);
        }
        for (auto &e: vFormerEdges) {
            formerEdges.push_back(e);
        }
        std::unordered_set<E> newEdges;
        for (auto &formerEdge: formerEdges) {
            if (formerEdge.getFrom() == u or formerEdge.getFrom() == v) {
                /* u or v was the start point of the edge -> s is now the startpoint*/
                newEdges.insert(E(s, formerEdge.getTo()));
            } else {
                /* u or v was the end point of the edge -> s is now the endpoint*/
                newEdges.insert(E(formerEdge.getFrom(), s));
            }
        }
        copy.removeNode(u);
        copy.removeNode(v);
        for (auto &newEdge: newEdges) {
            copy.addEdge(newEdge);
            if (Predicate(newEdge.getFrom(),newEdge.getTo())) {
                /*Push edges to queue if the new edge can be contracted too*/
                edgesToRemove.push(newEdge);
            }
        }
    }
}


static void benchmarkConnectedComponentsMerge(std::ofstream & outputFile, const WG & source, double distanceForMerge, u_int32_t workers = 1){
    static double maxDistance = distanceForMerge;
    struct MergePred{
        bool operator()(const XYNode & n1, const XYNode & n2)const{return n1.distanceTo(n2) <= maxDistance;}
    };
    {
        StopWatch w;
        auto result = contract(source,MergePred(),MergeFunction(),workers);
        outputFile << w.stop() <<";";
    }
}


static void benchmarkLegacy(std::ofstream & outputFile, const WG & source,double distanceForMerge){

        
    WG copy;
    for(auto & e: source.getEdges()){
        copy.addEdge(e);
    }
    static double maxDistance = distanceForMerge;
    struct MergePred{
        bool operator()(const XYNode & n1, const XYNode & n2)const{return n1.distanceTo(n2) <= maxDistance;}
    };
    std::function<bool(const XYNode &,const XYNode &)> pred = MergePred();
    {
        StopWatch w;
        legacyMerge(copy,pred);
        outputFile <<w.stop() <<";";
    }
}
static std::vector<std::pair<WG,double>> getTestSet(u_int32_t neighbours = 3, double mergePercentage = 0.1,u_int32_t maxNodesInExponenetOf10 = 4){
    std::vector<std::pair<WG,double>> dataSet;
    for(u_int32_t i = 1; i < maxNodesInExponenetOf10; i++){
        for(int j = 1; j < 10; j++) {
            int amountOfNodes = pow(10,i)*j;
            std::vector<XYNode> nodes = randomNodes(amountOfNodes);
            std::cout << "Constructing Test Graph with " << amountOfNodes << " nodes" << std::endl;
            WG graph{nodes};

            using E = WG::edge_type;
            std::vector<double> distances;
            distances.reserve(neighbours*amountOfNodes);
            for(auto & current : nodes){
                std::priority_queue<E, std::vector<E>, EdgeGreaterThan> candidates;
                for(auto & candidate : nodes){
                    if(current != candidate){
                        candidates.push(graph.makeEdge(current,candidate));
                    }
                }
                for(u_int32_t i = 0; i < neighbours; i++){
                    E e = candidates.top();
                    distances.push_back(e.getWeight());
                    candidates.pop();
                    graph.addEdge(e);
                }
            }
            std::sort(distances.begin(),distances.end());
            int indexOfMaxDistance = int((mergePercentage)*double(distances.size()-1));
            double maxDistance = distances[indexOfMaxDistance];
            dataSet.push_back(std::make_pair(std::move(graph),maxDistance));
        }
    }
    return dataSet;
}

static void testPerformanceConnectedComponents( int neighbours = 3,double mergePercentage = 0.1,int maxNodesInExponentOf10 = 4,u_int32_t maxWorkersExponentOf2 = 3){
    auto dataset = getTestSet(neighbours,mergePercentage,maxNodesInExponentOf10);
    std::string filename = "Multithreading_Performance_Analysis_Neighbours_"+std::to_string(neighbours)+"_MergePercentage_"+std::to_string(int(mergePercentage*100.0)) + ".csv";
    std::ofstream out{filename};
    out << "Procedure;";
    for(const auto & pair : dataset){
        out << pair.first.getNodes().size() << ";";
    }
    out << std::endl;
    for(u_int32_t i = 0; i <= maxWorkersExponentOf2; i++){
        u_int32_t workers = pow(2,i);
        out << "[ConnectedComponents Merge (Threads: "<< workers<<")];" ;
        for(auto & pair: dataset){
            benchmarkConnectedComponentsMerge(out,pair.first,pair.second,workers);
        }
        out << std::endl;
    }
    out.close();
}

static void comparePerformance( int neighbours = 3,double mergePercentage = 0.1,int maxNodesInExponentOf10 = 4,u_int32_t workers = 1){
    auto dataset = getTestSet(neighbours,mergePercentage,maxNodesInExponentOf10);

    std::string filename = "Performance_Comparision_Neighbours_"+std::to_string(neighbours)+"_MergePercentage_"+std::to_string(int(mergePercentage*100.0)) +"_Workers_"+std::to_string(workers)+ ".csv";
    std::ofstream out{filename};
    out << "Procedure;";
    for(const auto & pair: dataset) {
        out << pair.first.getNodes().size() << ";";
    }
    out << std::endl;
    out << "[ConnectedComponents Merge (Threads: "<< workers<<")];";
    for(auto & pair: dataset){
        benchmarkConnectedComponentsMerge(out,pair.first,pair.second,workers);
    }
    out << std::endl;
    out <<"[Legacy BruteForce Merge];" ;
    for (auto & pair: dataset) {
        benchmarkLegacy(out,pair.first,pair.second);
    }
    out << std::endl;
    out.close();
}




TEST(contractionPerformance, time){
    //testPerformanceConnectedComponents(5,0.2,4,3);
    testPerformanceConnectedComponents(3,0.1,4,3);
    //testPerformanceConnectedComponents(1,0.01,4,3);

    // comparePerformance(1,0.01,3);
    //comparePerformance(3,0.1,3);
    // comparePerformance(5,0.2,3);
}