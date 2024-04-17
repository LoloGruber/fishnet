// #ifndef Graph_H
// #define Graph_H
// #include <unordered_set>
// #include <unordered_map>
// #include "Graph.h"
// #include "UndirectedEdge.h"
// namespace graph{

// template<Node N,Annotation A,WeightFunction<N,A> W = EmptyWeightFunction<N,A>, HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
// class UndirectedGraph: public Graph<UndirectedGraph<N,A,W,Hash,Equal>,N,A,W,Hash,Equal,false>
// {

// public:
//     using EdgeType = UndirectedEdge<N,A,Hash,Equal>;
//     //using node_type = N;
//     using annotation_type = A;
//     using weight_function =  W;
//     using hash_function =  Hash;
//     using equality_function =  Equal;
//     using BaseGraph = Graph<UndirectedGraph,N,A,W,Hash,Equal,false>;
//     static constexpr bool isDirected = false;
// public:

//     UndirectedGraph():BaseGraph(){};

//     UndirectedGraph(std::vector<N> & nodes):BaseGraph(nodes){};


//     void removeNode(const N & node) {
//         auto adjList = this->getAdjListIterator(node);
//         if(adjList != this->adjMap.end()) {
//             for(auto & neighbour : this->getNeighbours(node)) {
//                 this->removeEdge(neighbour,node);
//             }
//             this->adjMap.erase(adjList);
//         }
//     }

//     void addEdge(N &  from, N &  to) {
//         this->addDirectedEdge(from,to);
//         this->addDirectedEdge(to,from);
//     }

//     void addEdge(N && from, N && to)  {
//         N f = from;
//         N t = to;
//         addEdge(f,t);
//     }

//     void addEdge( const EdgeType & edge) {
//         N f = edge.getFrom();
//         N t = edge.getTo();
//         addEdge(f,t);
//     }


//     bool containsEdge(const N & from, const N & to)const {
//         return this->containsNode(from) && this->containsNode(to) 
//         && this->containsDirectedEdge(from,to) && this->containsDirectedEdge(to,from);
//     }

//     bool containsEdge(const EdgeType & edge) const  {
//         return this->containsEdge(edge.getFrom(),edge.getTo());
//     }

//     void removeEdge(const N & from, const N & to) {
//         if(containsEdge(from,to)){
//             this->removeDirectedEdge(from, to);
//             this->removeDirectedEdge(to, from);
//         }
//     }

//     void removeEdge(const EdgeType & edge) {
//         removeEdge(edge.getFrom(),edge.getTo());
//     }

//     const BaseGraph::EdgeVector getEdges() const {
//         typename BaseGraph::EdgeVector edges;
//         auto nodeSet = std::unordered_set<N,Hash,Equal>();
//         for(auto const & pair : this->adjMap){
//             const N & from = pair.first;
//             nodeSet.insert(from);
//             for(auto const & to : pair.second){
//                 if(not nodeSet.contains(to)){
//                     edges.emplace_back(this->makeEdge(from,to));
//                 }
//             }
//         }
//         return edges;
//     }

//     const BaseGraph::NodeVector getReachableFrom(const N & node) const  {
//         return this->getNeighbours(node);
//     }

//     const typename BaseGraph::EdgeVector getInboundEdges(const N & node) const {
//         typename BaseGraph::EdgeVector inbound;
//         for(auto & outboundEdge: this->getOutboundEdges(node)){ //switch from and to of outbound edges (since graph is undirected: Inbound==Outbound)
//             inbound.emplace_back(this->makeEdge(outboundEdge.getTo(),outboundEdge.getFrom()));
//         }
//         return inbound;
//     }
//     ~UndirectedGraph() =default;




// };

// }

// #endif