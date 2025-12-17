// #ifndef DIRECTED_GRAPH_H
// #define DIRECTED_GRAPH_H
// #include "Graph.h"
// #include "DirectedEdge.h"
// namespace graph{
// template<Node N, Annotation A,WeightFunction<N,A> W = EmptyWeightFunction<N,A>, HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
// class DirectedGraph:public Graph<DirectedGraph<N,A,W,Hash,Equal>,N,A,W,Hash,Equal,true>
// {
// public:
//     using EdgeType = DirectedEdge<N,A,Hash,Equal>;
//     using node_type = N;
//     using annotation_type = A;
//     using weight_function =  W;
//     using hash_function =  Hash;
//     using equality_function =  Equal;
//     using BaseGraph = Graph<DirectedGraph,N,A,W,Hash,Equal,true>;
//     static constexpr bool isDirected = true;


//     auto getAdjListIterator(const N & node){
//         return std::find_if(this->adjMap.begin(),this->adjMap.end(),[&node,this](const auto & pair){return this->equalityFunction(pair.first,node);});
//     }

// public:
//     DirectedGraph():BaseGraph(){};
//     DirectedGraph(std::vector<N> & nodes):BaseGraph(nodes){};


//     void removeNode(const N & node) {
//         auto adjList = this->getAdjListIterator(node);
//         if (adjList != this->adjMap.end()) {
//             for(auto & neighbour: this->getReachableFrom(node)){
//                 this->removeEdge(neighbour,node);
//             }
//             this->adjMap.erase(adjList);
//         }
//     }

//     void addEdge(N & from, N & to) {
//         this->addDirectedEdge(from,to);
//     }


//     void addEdge( const EdgeType & edge) {
//         N f = edge.getFrom();
//         N t = edge.getTo();
//         addEdge(f,t);
//     }


//     bool containsEdge(const N & from, const N & to) const {
//         return this->containsNode(from) && this->containsNode(to) 
//         && std::any_of(this->adjMap.at(from).begin(),this->adjMap.at(from).end(),[&to,this](const N & current){return this->equalityFunction(current,to);});
//     }

//     bool containsEdge(const EdgeType & edge) const {
//         return this->containsEdge(edge.getFrom(),edge.getTo());
//     }

//     void removeEdge(const N & from, const N & to)  {
//         auto adjListIterator = getAdjListIterator(from);
//         if(adjListIterator != this->adjMap.end()){
//             this->adjMap.at(from).erase(std::remove(this->adjMap.at(from).begin(),this->adjMap.at(from).end(),to));
//         }    
//     }

//     void removeEdge(const EdgeType & edge) {
//         removeEdge(edge.getFrom(),edge.getTo());
//     }

//     const BaseGraph::EdgeVector getEdges() const {
//         typename BaseGraph::EdgeVector edges;
//         for(auto const & pair: this->adjMap) {
//             for(auto const & to: pair.second) {
//                 edges.emplace_back(this->makeEdge(pair.first,to));
//             }
//         }
//         return edges;
//     }


//     const BaseGraph::NodeVector getReachableFrom(const N & node) const {
//         typename BaseGraph::NodeVector reachable;
//         for(auto const& pair: this->adjMap){
//             if(this->containsEdge(pair.first,node)) {
//                 reachable.push_back(pair.first);
//             }
//         }
//         return reachable;
//     }


//     const BaseGraph::EdgeVector getInboundEdges(const N & node) const  {
//         typename BaseGraph::EdgeVector inbound;
//         for(auto const & pair : this->adjMap) {
//             if(this->containsEdge(pair.first,node)) {
//                inbound.emplace_back(this->makeEdge(pair.first,node));
//             }
//         }
//         return inbound;
//     }

//     ~DirectedGraph() = default;
// };
// }

// #endif