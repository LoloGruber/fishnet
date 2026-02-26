#include <fishnet/AdjacencyContainerDecorator.hpp>
#include <fishnet/AdjacencyMap.hpp>

template<typename N> requires fishnet::util::Hashable<N> && std::equality_comparable<N>
class MoveOnlyAdjacency: public fishnet::graph::AdjacencyContainerDecorator<fishnet::graph::AdjacencyMap<N>> {
public:
    using Base = fishnet::graph::AdjacencyContainerDecorator<fishnet::graph::AdjacencyMap<N>>;
    using node_type = typename Base::node_type;
    using equality_predicate = typename Base::equality_predicate;
    using hash_function = typename Base::hash_function;

    MoveOnlyAdjacency(): Base(fishnet::graph::AdjacencyMap<N>()) {}

    MoveOnlyAdjacency(const MoveOnlyAdjacency &) = delete;
    MoveOnlyAdjacency & operator=(const MoveOnlyAdjacency &) = delete;

    MoveOnlyAdjacency(MoveOnlyAdjacency &&) = default;
    MoveOnlyAdjacency & operator=(MoveOnlyAdjacency &&) = default;
};

template<typename N> requires fishnet::util::Hashable<N> && std::equality_comparable<N>
class MoveOnlyNoDefaultConstructorAdjacency: public MoveOnlyAdjacency<N>{
public:
    using Base = MoveOnlyAdjacency<N>;
    using node_type = typename Base::node_type;
    using equality_predicate = typename Base::equality_predicate;
    using hash_function = typename Base::hash_function; 
    int value;

    MoveOnlyNoDefaultConstructorAdjacency() = delete;
    MoveOnlyNoDefaultConstructorAdjacency(int value): Base(), value(value) {}
};

static_assert(fishnet::graph::AdjacencyContainer<MoveOnlyAdjacency<int>,int>);
static_assert(fishnet::graph::AdjacencyContainer<MoveOnlyNoDefaultConstructorAdjacency<int>,int>);