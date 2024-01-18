#include <vector>
#include <iostream>
#include <functional>
#include <concepts>
#include <optional>



template<typename F,typename T>
concept BiPredicate = std::convertible_to<F,std::function<bool( T ,  T )>>;

template<typename F, typename T>
concept BiOperator= std::convertible_to<F,std::function<T(const T &,const T &)>>;

template<typename F, typename T, typename R>
concept BiFunction = std::convertible_to<F, std::function<R(const T &, const T &)>>;

template<typename T>
bool cmp(BiPredicate<T> auto f, T a, T b){
    return f(a,b);
}

template<typename T>
T add(BiOperator<T> auto f, T a, T b){
    return f(a,b);
}


template<class E,typename N=E::node_type>
concept BaseEdge =  requires(const E & e, const E & o,  const N & n){
    {e.getFrom()} -> std::same_as<const N &>;
    {e.getTo()} -> std::same_as<const N &>;
    {e == o} -> std::convertible_to<bool>;
    {E(n,n)};
};

template<class E, typename N= E::node_type, typename A=E::annotation_type, typename W=E::weight_function>
concept WeightedEdge = BaseEdge<E,N> && requires (const E & e, const N & n, const A & a){
    {e.getWeight()} -> std::convertible_to<A>;
    {E(n,n,a)};
};

template<typename G, typename N=G::node_type, typename A=G::annotation_type,typename Edge=G::edge_type>
concept BaseGraph = BaseEdge<Edge,N> && requires ( G & g, N & nodeRef, N && nodeRval, const N & cNodeRef){
    {g.addNode(nodeRef)};
    {g.addNode(nodeRval)};
    {g.addEdge(nodeRef,nodeRef)};
    {g.getEdges()} -> std::convertible_to<const std::vector<Edge>>;
    {g.makeEdge(cNodeRef,cNodeRef)} -> std::convertible_to<Edge>;
};

template<typename G, typename N=G::node_type, typename A=G::annotation_type, typename Edge=G::edge_type, typename W=Edge::weight_function>
concept WeightedGraph = BaseGraph<G,N,A,Edge> && WeightedEdge<Edge,N,A,W>;


template<typename N,typename A,typename Hash = std::hash<N>,typename Equal = std::equal_to<N>>
class Edge{
protected:
    N from;
    N to;
    static inline Equal eq = Equal();
public:

    using node_type = N;
    using annotation_type = A;

    Edge(const N & from, const N & to):from(from),to(to){}

    const N & getFrom()const {
        return from;
    }

    const N & getTo() const{
        return to;
    }


    bool operator==(const Edge<N,A,Hash,Equal> & other)const{
        return eq(from, other.getFrom()) and eq(to, other.getTo());
    }
};

template<typename N,typename A, BiFunction<N,A> W,typename Hash= std::hash<N>, typename Equal = std::equal_to<N>>
class WeightedEdgeImpl: public Edge<N,A,Hash,Equal>{
private:
    A weight;
public:
    WeightedEdgeImpl(const N & from, const N & to, const A & weight): Edge<N,A,Hash,Equal>(from,to),weight(weight){};

    WeightedEdgeImpl(const N & from, const N & to):Edge<N,A,Hash,Equal>(from,to){
        this->weight = W()(from,to);
    }

    const A& getWeight()const{
        return weight;
    }
};

template<typename GraphImpl,typename N,typename A,typename E>
class AbstractGraph {
// requires BaseGraph<AbstractGraph<GraphImpl>,N,A,E> 
protected:
    std::vector<E> edges;

public:

    using node_type=N;
    using annotation_type=A;
    using edge_type=E;
    void addNode(N & node){
        static_cast<GraphImpl&>(*this).addNode(node);
    }

    void addNode(N && node) {

    }

    void addEdge(N & from, N & to){
        static_cast<GraphImpl&>(*this).addEdge(from,to);
    }

    const std::vector<E> getEdges(){
        return edges;
    }

    E makeEdge(const N & from, const N & to){
        return E(from,to);
    }
};

template<typename N,typename A, typename E> requires BaseEdge<E,N>
class MyGraph : public AbstractGraph<MyGraph<N,A,E>,N,A,E> {
public:
    void addNode(N & node){
        std::cout << "adding Node"<< std::endl;
    }

    void addNode(N && node){

    }

    void addEdge(N & from, N & to){
        this->edges.emplace_back(E(from,to));
        std::cout << "adding Edge" << std::endl;
    }

};

template<typename N,typename A>
using TestGraph = AbstractGraph<MyGraph<N,A,Edge<N,A>>,N,A,Edge<N,A>>;

template<typename N, typename A, typename W>
using TestWeightedGraph = AbstractGraph<MyGraph<N,A,WeightedEdgeImpl<N,A,W>>,N,A,WeightedEdgeImpl<N,A,W>>;


template<typename G> requires BaseGraph<G>
static void addEdgeAndGet(G & g, typename G::node_type & from, typename G::node_type & to){
    g.addEdge(from,to);
    g.addEdge(from,to);
    auto edges = g.getEdges();
    auto test = g.makeEdge(from,to);
    std::cout << (edges[0] == test) << std::endl;
    std::cout << (edges[0]==edges[1]) << std::endl;
}

template<typename G>
static void test(G & g){
    typename G::nodetype x;
}

static void getEdges(BaseGraph auto & graph){
    graph.getEdges();
}

template<typename G> requires WeightedGraph<G>
typename G::edge_type makeWeightedEdge(G & g, typename G::node_type const& from, typename G::node_type const& to){
    return g.makeEdge(from,to);
}






